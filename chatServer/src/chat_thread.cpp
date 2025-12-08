#include "chat_thread.h"
#include <sstream>
#include "log.h"
#include <filesystem>

// Base64解码函数
static std::string base64_decode(const std::string& encoded) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string decoded;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T[base64_chars[i]] = i;
    }

    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (c == '=') break;
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return decoded;
}
ChatThread::ChatThread() : base(event_base_new()),
                           _thread(std::make_unique<std::thread>(worker, this)),
                           _id(_thread->get_id())
{
    // 初始化文件存储目录
    init_file_storage();
}

ChatThread::~ChatThread()
{
}

void ChatThread::start(std::shared_ptr<ChatInfo> &info, std::shared_ptr<DataBase> &db)
{
    this->info = info;
    this->db = db;
}

void ChatThread::worker(ChatThread *t)
{
    t->run();
}

void ChatThread::run()
{
    // 创建一个定时器事件
    struct event *timeout = event_new(base.get(), -1, EV_PERSIST, timeout_cb, this);
    if (!timeout)
    {
        LOG_PERROR("event_new");
        return;
    }

    struct timeval tv;
    // 给tv变量清0
    evutil_timerclear(&tv);
    tv.tv_sec = 3;
    // 将事件添加到base中
    event_add(timeout, &tv);
    std::cout << "--- thread " << _id << " start working ..." << std::endl;
    // 开始执行事件
    event_base_dispatch(base.get());

    // 回收事件集合
    base.reset();
}

void ChatThread::timeout_cb(evutil_socket_t fd, short event, void *arg)
{
    ChatThread *t = (ChatThread *)arg;
    // std::cout<<"thread"<<t->get_id()<<" is listening"<<std::endl;
}

std::thread::id ChatThread::thread_get_id()
{
    return _id;
}

struct event_base *ChatThread::thread_get_base()
{
    return base.get();
}

void ChatThread::thread_read_cb(struct bufferevent *bev, void *ctx)
{
    ChatThread *t = (ChatThread *)ctx;

    // 循环处理缓冲区中的所有数据包
    while (true)
    {
        // 检查是否有足够的数据读取长度字段
        struct evbuffer *input = bufferevent_get_input(bev);
        size_t available = evbuffer_get_length(input);
        int packet_size = 0; // 数据包大小
        evbuffer_copyout(input, &packet_size, 4);

        // 解决数据包过大时分包问题
        if (available < 4 + packet_size)
        {
            break; // 没有足够的数据读取长度，等待下次回调
        }
        std::cout << "packet_size:" << packet_size << std::endl;

        if (packet_size <= 0 || packet_size > MAX_PACKET_SIZE)
        {
            LOG_ERROR("packet_size error");
            return;
        }

        // 动态缓冲区
        std::vector<char> buf(packet_size + 1);
        memset(buf.data(), 0, buf.size());

        // 尝试读取一个完整的数据包
        if (!t->thread_read_data(bev, buf.data(), buf.size()))
        {
            LOG_PERROR("thread_read_data");
            return;
        }

        std::cout << "--thread " << t->thread_get_id() << " receive data " << buf.data() << std::endl;

        // 解析json数据
        Json::Reader reader; // json 解析对象
        Json::Value val;     // json存储的容器 类似于map

        if (!reader.parse(buf.data(), val))
        {
            LOG_PERROR("parse");
            return;
        }
        // 处理注册事件
        if (val["cmd"] == "register")
        {
            // std::cout<<"客户端注册"<<std::endl;
            t->thread_register(bev, val);
        }
        // 处理登录事件
        else if (val["cmd"] == "login")
        {
            t->thread_login(bev, val);
        }
        // 处理添加好友事件
        else if (val["cmd"] == "addfriend")
        {
            t->thread_add_friend(bev, val);
        }
        // 处理私聊事件
        else if (val["cmd"] == "private")
        {
            t->thread_private_chat(bev, val);
        }
        // 处理添加群组事件
        else if (val["cmd"] == "creategroup")
        {
            t->thread_create_group(bev, val);
        }
        // 处理加入群聊事件
        else if (val["cmd"] == "joingroup")
        {
            t->thread_join_group(bev, val);
        }
        // 处理群聊事件
        else if (val["cmd"] == "groupchat")
        {
            t->thread_group_chat(bev, val);
        }
        // 处理文件传输事件
        else if (val["cmd"] == "file")
        {
            t->thread_transer_file(bev, val);
        }
        // 处理私聊图片消息
        else if (val["cmd"] == "image_private")
        {
            t->thread_image_private_chat(bev, val);
        }
        // 处理群聊图片消息
        else if (val["cmd"] == "image_group")
        {
            t->thread_image_group_chat(bev, val);
        }
        // 处理文件传输消息
        else if (val["cmd"] == "transfer_file")
        {
            t->thread_transer_file(bev, val);
        }
        // 处理客户端下线事件
        else if (val["cmd"] == "offline")
        {
            t->thread_client_offline(bev, val);
        }
        // 处理查询用户uid事件
        else if (val["cmd"] == "query_by_uid")
        {
            t->thread_query_user_by_uid(bev, val);
        }
        // 处理模糊查询事件
        else if (val["cmd"] == "fuzzy_search")
        {
            t->thread_query_fuzzy_search(bev, val);
        }
        else if (val["cmd"] == "query_by_gid")
        {
            t->thread_query_group_by_gid(bev, val);
        }
        else if(val["cmd"]=="query_users_batch")
        {
            t->thread_query_users_by_uids_batch(bev,val);
        }
    }
}

void ChatThread::thread_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    ChatThread *t = static_cast<ChatThread *>(ctx);
    // 客户端连接已关闭
    if (events & BEV_EVENT_EOF)
    {
        std::cout << "[disconnect] client offline" << std::endl;
        // 删除链表节点
        t->thread_eventcb_list_delete(bev);
        // 释放事件
        bufferevent_free(bev);
    }
    else
    {
        std::cout << "Unknown Error" << std::endl;
        // 删除链表节点
        t->thread_eventcb_list_delete(bev);
        // 释放事件
        bufferevent_free(bev);
    }
}

bool ChatThread::thread_read_data(struct bufferevent *bev, char *s, size_t buffer_size)
{
    // 要读取的长度
    int sz = 0;
    // // 先读取长度，放入sz中
    if (bufferevent_read(bev, &sz, 4) != 4)
    {
        LOG_PERROR("bufferevent_read length");
        return false;
    }
    // // 检查数据长度是否合理
    if (sz <= 0 || sz > MAX_PACKET_SIZE) // 限制最大10KB
    {
        LOG_ERROR("Invalid data size");
        return false;
    }

    // // 检查目标缓冲区是否有足够空间
    if (sz >= buffer_size)
    {
        LOG_ERROR("Buffer too small for data size");
        return false;
    }

    size_t count = 0;
    // 临时缓冲区
    char buf[4096] = {0};

    while (count < sz)
    {
        // 计算本次需要读取的字节数
        size_t bytes_to_read = sz - count;
        if (bytes_to_read > sizeof(buf))
            bytes_to_read = sizeof(buf);

        // 读取数据
        ssize_t bytes_read = bufferevent_read(bev, buf, bytes_to_read);
        if (bytes_read <= 0)
        {
            LOG_PERROR("bufferevent_read data");
            return false;
        }

        // 安全地拷贝数据到目标缓冲区
        size_t remaining_space = buffer_size - count - 1; // -1 for null terminator
        if (bytes_read > remaining_space)
        {
            LOG_ERROR("Buffer overflow prevented");
            return false;
        }

        memcpy(s + count, buf, bytes_read);
        count += bytes_read;

        // 清空临时缓冲区
        memset(buf, 0, sizeof(buf));
    }

    // 确保字符串以null结尾
    s[count] = '\0';
    return true;
}

void ChatThread::thread_register(struct bufferevent *bev, Json::Value &v)
{
    // 连接数据库
    if (db->database_connect() == false)
    {
        LOG_PERROR("database_connect");
        return;
    }
    // // 判断用户是否已存在

    // // 用户已存在
    // if (db->database_user_is_exist(v["username"].asString()))
    // {
    //     Json::Value val;
    //     val["cmd"] = "register_reply";
    //     val["result"] = "user_exist";
    //     // 发送回应给客户端
    //     thread_write_data(bev, val);
    // }

    // 用户不存在
    // else
    // {
    // 将用户添加到数据库中，会自动生成UID
    db->database_insert_user_info(v);

    // 获取新生成的UID
    std::string username = v["username"].asString();
    char sql[256];
    sprintf(sql, "SELECT uid FROM chat_user WHERE username='%s';", username.c_str());
    MYSQL_ROW row;
    uint64_t new_uid = 0;

    // 查询UID（数据库已经连接）
    if (db->exec_query_and_fetch_row(sql, row))
    {
        new_uid = std::stoull(row[0]);
    }

    // 发送回应给客户端
    Json::Value val;
    val["cmd"] = "register_reply";
    val["result"] = "success";
    val["uid"] = Json::Value::UInt64(new_uid);
    val["username"] = username;
    thread_write_data(bev, val);
    // }

    // 断开数据库
    db->database_disconnect();
}

void ChatThread::thread_write_data(struct bufferevent *bev, const Json::Value &v)
{
    // 封装json数据为字符串
    Json::FastWriter writer;
    std::string val = writer.write(v);
    int len = val.size();
    size_t total_size = 4 + len;
    std::vector<char> wbuf(total_size);
    memcpy(&wbuf[0], &len, 4);
    memcpy(&wbuf[0] + 4, val.c_str(), len);
    // 将数据写入输出缓冲区中
    bufferevent_write(bev, &wbuf[0], len + 4);
}

void ChatThread::thread_login(struct bufferevent *bev, Json::Value &v)
{
    // 连接数据库
    if (db->database_connect() == false)
    {
        LOG_PERROR("database_connect");
        return;
    }

    uint64_t loginUid = 0;
    std::string loginName;

    if (v.isMember("uid") && v["uid"].isUInt64())
    {
        // UID登录
        loginUid = v["uid"].asUInt64();
        // 用户不存在
        if (!db->database_user_is_exist_by_uid(loginUid))
        {
            Json::Value val;
            val["cmd"] = "login_reply";
            val["result"] = "not_exist";
            thread_write_data(bev, val);
            db->database_disconnect();
            return;
        }
    }

    // 用户存在，判断密码是否正确
    bool password_correct;
    if (loginUid > 0)
    {
        password_correct = db->database_password_correct_by_uid(loginUid, v["password"].asString());
    }

    // 密码不正确
    if (!password_correct)
    {
        Json::Value val;
        val["cmd"] = "login_reply";
        val["result"] = "password_error";
        thread_write_data(bev, val);
        db->database_disconnect();
        return;
    }

    // 读取好友列表与群组列表
    std::string friendlist, grouplist;
    bool get_friend_group_success;

    if (loginUid > 0)
    {
        get_friend_group_success = db->database_get_friend_group_by_uid(loginUid, friendlist, grouplist);
    }
    if (!get_friend_group_success)
    {
        LOG_PERROR("database_get_friend_group");
        db->database_disconnect();
        return;
    }
    loginName = db->database_get_username_by_uid(loginUid);
    // 回复客户端登陆成功
    Json::Value Val;
    Val["cmd"] = "login_reply";
    Val["result"] = "success";
    Val["uid"] = loginUid > 0 ? Json::Value::UInt64(loginUid) : Json::Value::null;
    Val["username"] = loginName;
    Val["friendlist"] = friendlist;
    Val["grouplist"] = grouplist;
    thread_write_data(bev, Val);

    // 更新在线用户链表 - 需要传递UID信息
    Json::Value user_info;
    user_info["uid"] = loginUid > 0 ? Json::Value::UInt64(loginUid) : Json::Value::null;
    user_info["username"] = loginName;
    info->list_update_list(bev, user_info);

    // 发送上线提示给所有好友
    if (friendlist.empty())
    {
        db->database_disconnect();
        return;
    }

    // 遍历好友列表，给在线的用户发送信息
    std::string Friend[1024];
    int num = thread_parse_string(friendlist, Friend);
    for (int i = 0; i < num; i++)
    {
        struct bufferevent *b = info->list_friend_online(Friend[i]);
        if (b == NULL)
        {
            db->database_disconnect();
            continue;
        }
        Val.clear();
        Val["cmd"] = "online";
        Val["uid"] = loginUid > 0 ? Json::Value::UInt64(loginUid) : Json::Value::null;
        Val["username"] = loginName;
        thread_write_data(b, Val);
    }

    // 断开与数据库的连接
    db->database_disconnect();
}

int ChatThread::thread_parse_string(std::string &f, std::string *s)
{
    int count = 0, start = 0;
    int idx = f.find('|');
    while (idx != -1)
    {
        s[count++] = f.substr(start, idx - start);
        start = idx + 1;
        idx = f.find('|', start);
    }
    s[count++] = f.substr(start);
    return count;
}

void ChatThread::thread_add_friend(struct bufferevent *bev, const Json::Value &v)
{
    // 连接数据库
    if (!db->database_connect())
    {
        LOG_PERROR("database_connect");
        return;
    }

    Json::Value val;

    // 检查是否提供了friend_uid字段
    if (!v.isMember("friend_uid") || !v["friend_uid"].isUInt64())
    {
        val["cmd"] = "addfriend_reply";
        val["request_id"] = v["request_id"];
        val["result"] = "invalid_request";
        thread_write_data(bev, val);
        db->database_disconnect();
        return;
    }

    uint64_t friend_uid = v["friend_uid"].asUInt64();
    uint64_t current_uid = v["uid"].asUInt64();

    // 检查用户是否存在
    if (!db->database_user_is_exist_by_uid(friend_uid))
    {
        val["cmd"] = "addfriend_reply";
        val["request_id"] = v["request_id"];
        val["result"] = "not_exist";
        thread_write_data(bev, val);
        db->database_disconnect();
        return;
    }

    // 获取好友用户名用于回复
    std::string friend_name = db->database_get_username_by_uid(friend_uid);

    // 获取当前用户的好友列表
    std::string Friend[1024];
    std::string friendlist, grouplist;

    if (db->database_get_friend_group_by_uid(current_uid, friendlist, grouplist))
    {
        if (!friendlist.empty())
        {
            int num = thread_parse_string(friendlist, Friend);
            for (int i = 0; i < num; i++)
            {
                if (std::stoull(Friend[i]) == friend_uid)
                {
                    val["cmd"] = "addfriend_reply";
                    val["request_id"] = v["request_id"];
                    val["result"] = "already_friend";
                    thread_write_data(bev, val);
                    db->database_disconnect();
                    return;
                }
            }
        }
    }

    // 准备添加好友的JSON数据
    Json::Value add_friend_request;
    add_friend_request["uid"] = Json::Value::UInt64(current_uid);
    add_friend_request["friend_uid"] = Json::Value::UInt64(friend_uid);

    // 修改数据库
    db->database_add_friend(add_friend_request);

    // 回复好友
    val["cmd"] = "be_addfriend";
    val["uid"] = Json::Value::UInt64(current_uid);
    val["username"] = v["username"];
    struct bufferevent *b = info->list_friend_online(std::to_string(friend_uid));
    if (b != NULL)
        thread_write_data(b, val);

    // 回复客户端
    val.clear();
    val["cmd"] = "addfriend_reply";
    val["result"] = "success";
    val["friend_uid"] = Json::Value::UInt64(friend_uid);
    val["friend_username"] = friend_name;
    val["request_id"] = v["request_id"];
    thread_write_data(bev, val);

    db->database_disconnect();
}

void ChatThread::thread_private_chat(struct bufferevent *bev, const Json::Value &v)
{
    uint64_t userUid = v["uid"].asUInt64();
    uint64_t friendUid = v["tofriend"].asUInt64();
    struct bufferevent *b = info->list_friend_online(std::to_string(friendUid));
    Json::Value val;
    if (v.isMember("request_id"))
        val["request_id"] = v["request_id"];
    // 如果好友不在线
    if (b == NULL)
    {
        val["cmd"] = "private_reply";
        val["result"] = "offline";
        thread_write_data(bev, val);
        return;
    }
    // 好友在线
    val["cmd"] = "private";
    val["fromfriend"] = Json::Value::UInt64(userUid);
    val["text"] = v["text"];
    thread_write_data(b, val);
}

void ChatThread::thread_create_group(struct bufferevent *bev, const Json::Value &v)
{
    std::string groupname = v["ownername"].asString() + "的小群";
    // 群成员
    std::string groupMember = v["group_member"].asString();

    // 群主id
    uint64_t owner_id = v["owner"].asUInt64();
    std::string owner = std::to_string(owner_id);

    Json::Value val;

    // 解析群成员
    std::string member[1024];
    int memberCount = thread_parse_string(groupMember, member);

    // 在数据库中添加新群
    if (!db->database_connect())
    {
        LOG_ERROR("database_connect");
        return;
    }

    uint64_t gid = db->database_add_new_group(groupname, owner);

    if (gid == 0)
    {
        LOG_ERROR("Failed to create group");
        db->database_disconnect();
        return;
    }

    // 将新群添加到群信息中
    info->list_add_new_group(gid, owner);

    // 将所有成员添加到群中（包括群主）
    std::string gid_str = std::to_string(gid);

    // 更新群主的群组列表
    db->database_update_grouplist_by_uid(owner_id, gid_str);

    // 再添加其他成员
    for (int i = 0; i < memberCount; i++)
    {
        // 如果成员不是群主，则添加到群中
        if (member[i] != owner)
        {
            db->database_update_group_member_by_gid(gid, member[i]);
            info->list_update_group_member_by_gid(gid, member[i]);
            // 更新成员的群组列表
            uint64_t member_uid = std::stoull(member[i]);
            db->database_update_grouplist_by_uid(member_uid, gid_str);
        }
    }

    db->database_disconnect();

    val["cmd"] = "creategroup_reply";
    val["result"] = "success";
    val["gid"] = Json::Value::UInt64(gid);
    val["groupmember"] = groupMember;
    val["request_id"] = v["request_id"];
    val["groupname"] = groupname;

    // 向所有群成员发送群信息
    for (int i = 0; i < memberCount; i++)
    {
        struct bufferevent *bvt = info->list_friend_online(member[i]);
        if (bvt != nullptr)
            thread_write_data(bvt, val);
    }
}

void ChatThread::thread_join_group(struct bufferevent *bev, const Json::Value &v)
{
    uint64_t gid = v["gid"].asUInt64();
    uint64_t uid = v["uid"].asUInt64();
    Json::Value val;
    val["request_id"]=v["request_id"];
    // 检查群是否存在
    if (!db->database_connect())
    {
        LOG_ERROR("database_connect");
        val["cmd"] = "joingroup_reply";
        val["result"] = "db_error";
        thread_write_data(bev, val);
        return;
    }
    
    // 检查群是否存在
    std::string groupname = db->database_get_groupname_by_gid(gid);
    db->database_disconnect();

    if (groupname.empty())
    {
        val["cmd"] = "joingroup_reply";
        val["result"] = "group_not_exist";
        thread_write_data(bev, val);
        return;
    }

    // 检查用户是否存在
    if (!db->database_connect())
    {
        LOG_ERROR("database_connect");
        val["cmd"] = "joingroup_reply";
        val["result"] = "db_error";
        thread_write_data(bev, val);
        return;
    }
    
    if (!db->database_user_is_exist_by_uid(uid))
    {
        val["cmd"] = "joingroup_reply";
        val["result"] = "user_not_exist";
        thread_write_data(bev, val);
        db->database_disconnect();
        return;
    }
    
    // 获取用户名
    std::string username = db->database_get_username_by_uid(uid);
    db->database_disconnect();

    if (username.empty())
    {
        val["cmd"] = "joingroup_reply";
        val["result"] = "user_not_exist";
        thread_write_data(bev, val);
        return;
    }

    // 判断群中是否已有当前成员
    if (info->list_member_is_group_by_gid(gid, std::to_string(uid)))
    {
        val["cmd"] = "joingroup_reply";
        val["result"] = "already_member";
        thread_write_data(bev, val);
        return;
    }

    // 修改数据库 - 添加成员到群组
    db->database_connect();
    db->database_update_group_member_by_gid(gid, std::to_string(uid));
    db->database_update_grouplist_by_uid(uid, std::to_string(gid));
    db->database_disconnect();

    // 更新链表中群信息
    info->list_update_group_member_by_gid(gid, std::to_string(uid));

    // 向群中所有用户发送消息
    std::list<std::string> l = info->list_get_list_by_gid(gid);
    std::string member_str;
    for (auto it = l.begin(); it != l.end(); it++)
    {
        if (*it == std::to_string(uid))
            continue;
        member_str.append(*it);
        member_str.append("|");
        struct bufferevent *b = info->list_friend_online(*it);
        if (b == NULL)
            continue;
        val["cmd"] = "new_member_join";
        val["gid"] = Json::Value::UInt64(gid);
        val["groupname"] = groupname;
        val["uid"] = Json::Value::UInt64(uid);
        val["username"] = username;
        thread_write_data(b, val);
    }
    // 去掉最后的'|'
    if (!member_str.empty())
    {
        member_str.erase(member_str.size() - 1);
    }
    val.clear();
    // 向自己发送消息
    val["cmd"] = "joingroup_reply";
    val["request_id"]=v["request_id"];
    val["result"] = "success";
    val["gid"] = Json::Value::UInt64(gid);
    val["groupname"] = groupname;
    val["member"] = member_str;
    thread_write_data(bev, val);
}


void ChatThread::thread_group_chat(struct bufferevent *bev, const Json::Value &v)
{
    uint64_t gid = v["gid"].asUInt64();
    uint64_t uid = v["uid"].asUInt64();
    std::string text = v["text"].asString();
    
    // 获取发送者用户名
    if (!db->database_connect())
    {
        LOG_ERROR("database_connect");
        return;
    }
    
    std::string username = db->database_get_username_by_uid(uid);
    db->database_disconnect();
    
    if (username.empty())
    {
        LOG_ERROR("username not found for uid");
        return;
    }

    // 获取群组名称
    if (!db->database_connect())
    {
        LOG_ERROR("database_connect");
        return;
    }
    
    std::string groupname = db->database_get_groupname_by_gid(gid);
    db->database_disconnect();

    // 获取成员列表
    std::list<std::string> member = info->list_get_list_by_gid(gid);
    // 将消息转发给群成员
    for (auto it = member.begin(); it != member.end(); it++)
    {
        if (*it == std::to_string(uid))
            continue;
        struct bufferevent *b = info->list_friend_online(*it);
        if (b == NULL)
            continue;
        Json::Value val;
        val["cmd"] = "groupchat_reply";
        val["gid"] = Json::Value::UInt64(gid);
        val["groupname"] = groupname;
        val["from_uid"] = Json::Value::UInt64(uid);
        val["from_username"] = username;
        val["text"] = text;
        thread_write_data(b, val);
    }
}

void ChatThread::thread_transer_file(struct bufferevent *bev, const Json::Value &v)
{
    // 获取文件信息
    uint64_t userUid = v["uid"].asUInt64();
    std::string fileId = v["file_id"].asString();
    std::string fileData = v["file_data"].asString();
    std::string fileName = v["file_name"].asString();
    int fileSize = v["file_size"].asInt();

    Json::Value val;
    if (v.isMember("request_id"))
        val["request_id"] = v["request_id"];

    // 判断是私聊还是群聊
    bool isGroupChat = v.isMember("gid");
    uint64_t targetId = 0;

    if (isGroupChat) {
        // 群聊文件传输
        targetId = v["gid"].asUInt64();

        // 获取发送者用户名
        if (!db->database_connect())
        {
            LOG_ERROR("database_connect");
            return;
        }

        std::string username = db->database_get_username_by_uid(userUid);
        db->database_disconnect();

        if (username.empty())
        {
            LOG_ERROR("username not found for uid");
            return;
        }

        // 获取群组名称
        if (!db->database_connect())
        {
            LOG_ERROR("database_connect");
            return;
        }

        std::string groupname = db->database_get_groupname_by_gid(targetId);
        db->database_disconnect();

        // 获取成员列表并发送文件消息
        std::list<std::string> member = info->list_get_list_by_gid(targetId);
        for (auto it = member.begin(); it != member.end(); it++)
        {
            // 不发送给自己
            if (*it == std::to_string(userUid))
                continue;

            struct bufferevent *memberBev = info->list_friend_online(*it);
            if (memberBev == NULL)
                continue;

            Json::Value fileVal;
            fileVal["cmd"] = "transfer_file";
            fileVal["gid"] = Json::Value::UInt64(targetId);
            fileVal["groupname"] = groupname;
            fileVal["from_uid"] = Json::Value::UInt64(userUid);
            fileVal["from_username"] = username;
            fileVal["file_id"] = fileId;
            fileVal["file_data"] = fileData;
            fileVal["file_name"] = fileName;
            fileVal["file_size"] = fileSize;
            thread_write_data(memberBev, fileVal);
        }

        // 回复发送者成功
        val["cmd"] = "transfer_file_reply";
        val["result"] = "success";
        thread_write_data(bev, val);
    }
    else {
        // 私聊文件传输
        targetId = v["tofriend"].asUInt64();

        // 查找好友是否在线
        struct bufferevent *friendBev = info->list_friend_online(std::to_string(targetId));

        // 好友不在线
        if (friendBev == NULL)
        {
            val["cmd"] = "transfer_file_reply";
            val["result"] = "offline";
            thread_write_data(bev, val);
            return;
        }

        // 好友在线，发送文件消息
        val["cmd"] = "transfer_file";
        val["fromfriend"] = Json::Value::UInt64(userUid);
        val["file_id"] = fileId;
        val["file_data"] = fileData;
        val["file_name"] = fileName;
        val["file_size"] = fileSize;
        thread_write_data(friendBev, val);

        // 回复发送者成功
        val.clear();
        val["cmd"] = "transfer_file_reply";
        val["result"] = "success";
        if (v.isMember("request_id"))
            val["request_id"] = v["request_id"];
        thread_write_data(bev, val);
    }
}

void ChatThread::thread_client_offline(struct bufferevent *bev, const Json::Value &v)
{
    uint64_t uid = v["uid"].asUInt64();

    // 删除info的online_user中的客户端事件
    info->list_delete_user(std::to_string(uid));

    // 释放客户端事件
    bufferevent_free(bev);

    // 通知所有好友
    std::string friendlist, grouplist;
    db->database_connect();
    // 获取好友列表
    db->database_get_friend_group(v, friendlist, grouplist);

    db->database_disconnect();

    std::string Friend[1024];

    if (friendlist.empty())
        return;
    // 将好友列表解析到Friend数组
    int num = thread_parse_string(friendlist, Friend);
    // 通知所有好友
    for (int i = 0; i < num; i++)
    {
        struct bufferevent *b = info->list_friend_online(Friend[i]);
        if (b == NULL)
            continue;
        Json::Value val;
        val["cmd"] = "friend_offline";
        val["uid"] = Json::Value::UInt64(uid);
        thread_write_data(b, val);
    }
}

void ChatThread::thread_query_user_by_uid(struct bufferevent *bev, const Json::Value &v)
{
    Json::Value val;
    if (!v.isMember("uid"))
    {
        val["cmd"] = "invaild_request";
        thread_write_data(bev, val);
        return;
    }
    uint64_t uid = v["uid"].asUInt64();
    db->database_connect();
    if (!db->database_user_is_exist_by_uid(uid))
    {
        val["cmd"] = "not_exist";
        thread_write_data(bev, val);
        db->database_disconnect();
        return;
    }
    val["cmd"] = "query_uid_reply";
    val["uid"] = Json::Value::UInt64(uid);
    val["username"] = db->database_get_username_by_uid(uid);
    if (info->list_friend_online(std::to_string(uid)))
        val["status"] = "online";
    else
        val["status"] = "offline";
    val["request_id"] = v["request_id"];
    thread_write_data(bev, val);
    db->database_disconnect();
}

void ChatThread::thread_query_group_by_gid(struct bufferevent *bev, const Json::Value &v)
{
    Json::Value val;
    if (!v.isMember("gid"))
    {
        val["cmd"] = "invaild_request";
        thread_write_data(bev, val);
        return;
    }

    uint64_t gid = v["gid"].asUInt64();
    db->database_connect();
    // if (!db->database_user_is_exist_by_uid(uid))
    // {
    //     val["cmd"] = "not_exist";
    //     thread_write_data(bev, val);
    //     db->database_disconnect();
    //     return;
    // }

    val["cmd"] = "query_gid_reply";
    val["gid"] = Json::Value::UInt64(gid);
    val["groupname"] = db->database_get_groupname_by_gid(gid);
    val["groupmember"] = db->database_get_group_members_by_gid(gid);
    val["request_id"] = v["request_id"];
    thread_write_data(bev, val);
    db->database_disconnect();
}

void ChatThread::thread_query_fuzzy_search(struct bufferevent *bev, const Json::Value &v)
{
    Json::Value responce;
    Json::Value val; // json数组 存储数据
    std::string content = v["content"].asString();
    responce["cmd"] = "search_reply";
    responce["request_id"] = v["request_id"];
    db->database_connect();
    char sql[256] = {0};
    bool isGroup = (v["type"] == "group");
    std::stringstream query;
    query << "\'\%" << content << "\%\'";

    // 可优化：模糊查询
    if (!isGroup)
    {
        responce["type"] = "friend";
        sprintf(sql, "SELECT uid,username FROM chat_user\
        WHERE uid LIKE %s OR username LIKE %s;",
                query.str().c_str(), query.str().c_str());
    }

    else
    {
        responce["type"] = "group";
        sprintf(sql, "SELECT gid,groupname FROM chat_group\
        WHERE gid LIKE %s OR groupname LIKE %s;",
                query.str().c_str(), query.str().c_str());
    }
    std::vector<std::vector<std::string>> rows;
    int sz = db->exec_query_and_fetch_rows(sql, rows);

    if (sz == 0)
    {
        responce["data"] = "null";
        thread_write_data(bev, responce);
        db->database_disconnect();
        return;
    }

    // 对查找到的数据进行处理
    if (!isGroup)
    {
        for (int i = 0; i < sz; i++)
        {
            Json::Value row_data;
            row_data["uid"] = rows[i][0];
            row_data["username"] = rows[i][1];
            val.append(row_data);
        }
    }

    else
    {
        for (int i = 0; i < sz; i++)
        {
            Json::Value row_data;
            row_data["gid"] = rows[i][0];
            row_data["groupname"] = rows[i][1];
            val.append(row_data);
        }
    }

    responce["data"] = val;
    thread_write_data(bev, responce);
    db->database_disconnect();
}

void ChatThread::thread_query_users_by_uids_batch(struct bufferevent *bev, const Json::Value &v)
{
    Json::Value response;
    response["cmd"] = "query_users_batch_reply";
    
    // 检查是否有uids字段
    if (!v.isMember("uids") || !v["uids"].isArray())
    {
        response["error"] = "invalid_request";
        response["message"] = "Missing or invalid 'uids' field";
        thread_write_data(bev, response);
        return;
    }
    
    // 解析uid数组
    std::vector<uint64_t> uids;
    for (const auto& uid_val : v["uids"])
    {
        if (uid_val.isUInt64())
        {
            uids.push_back(uid_val.asUInt64());
        }
    }
    
    if (uids.empty())
    {
        response["error"] = "invalid_request";
        response["message"] = "No valid UIDs provided";
        thread_write_data(bev, response);
        return;
    }
    
    // 连接数据库
    db->database_connect();
    
    // 查询用户信息
    Json::Value users;
    if (db->database_get_users_by_uids(uids, users))
    {
        // 为每个用户添加在线状态
        for (auto& user : users)
        {
            uint64_t uid = user["uid"].asUInt64();
            if (info->list_friend_online(std::to_string(uid)))
            {
                user["status"] = "online";
            }
            else
            {
                user["status"] = "offline";
            }
        }
        response["users"] = users;
    }
    else
    {
        response["users"] = Json::Value(Json::arrayValue);
    }
    
    // 添加request_id（如果存在）
    if (v.isMember("request_id"))
    {
        response["request_id"] = v["request_id"];
    }
    
    // 发送响应
    thread_write_data(bev, response);
    db->database_disconnect();
}

void ChatThread::thread_eventcb_list_delete(struct bufferevent *bev)
{
    info->list_delete_user_by_bev(bev);
}

void ChatThread::init_file_storage()
{
    // 创建文件存储目录
    std::filesystem::create_directories(FILE_STORAGE_PATH);
}

std::string ChatThread::thread_save_file(const std::string& file_id, const std::string& file_data)
{
    // Base64解码
    std::string decoded = base64_decode(file_data);

    // 构建文件路径
    std::string file_path = std::string(FILE_STORAGE_PATH) + "/" + file_id + ".png";

    // 写入文件
    std::ofstream out(file_path, std::ios::binary);
    if (out.write(decoded.data(), decoded.size())) {
        return file_path;
    }

    return "";
}

void ChatThread::thread_send_chunked(struct bufferevent* bev, const std::string& file_id,
                                      const std::string& base64_data, const std::string& base_cmd,
                                      const Json::Value& extra_fields)
{
    size_t total_size = base64_data.size();
    size_t total_chunks = (total_size + CHUNK_SIZE - 1) / CHUNK_SIZE;

    std::cout << "[Chunked] Sending " << total_chunks << " chunks for file_id: " << file_id
              << ", total_size: " << total_size << std::endl;

    for (size_t i = 0; i < total_chunks; i++) {
        size_t offset = i * CHUNK_SIZE;
        size_t chunk_len = std::min((size_t)CHUNK_SIZE, total_size - offset);

        Json::Value chunk;
        chunk["cmd"] = base_cmd + "_chunk";
        chunk["file_id"] = file_id;
        chunk["chunk_index"] = (int)i;
        chunk["total_chunks"] = (int)total_chunks;
        chunk["file_size"] = (int)total_size;
        chunk["chunk_data"] = base64_data.substr(offset, chunk_len);

        // 添加额外字段
        for (const auto& key : extra_fields.getMemberNames()) {
            chunk[key] = extra_fields[key];
        }

        thread_write_data(bev, chunk);
    }
}

void ChatThread::thread_image_private_chat(struct bufferevent *bev, const Json::Value &v)
{
    uint64_t userUid = v["uid"].asUInt64();
    uint64_t friendUid = v["tofriend"].asUInt64();
    std::string fileId = v["file_id"].asString();
    std::string fileData = v["file_data"].asString();

    // 保存图片到服务器
    std::string filePath = thread_save_file(fileId, fileData);

    if (filePath.empty()) {
        // 保存失败，发送错误消息
        Json::Value errorVal;
        errorVal["cmd"] = "image_private_reply";
        errorVal["result"] = "save_failed";
        if (v.isMember("request_id"))
            errorVal["request_id"] = v["request_id"];
        thread_write_data(bev, errorVal);
        return;
    }

    // 查找好友是否在线
    struct bufferevent *friendBev = info->list_friend_online(std::to_string(friendUid));
    Json::Value val;
    if (v.isMember("request_id"))
        val["request_id"] = v["request_id"];

    // 好友不在线
    if (friendBev == NULL)
    {
        val["cmd"] = "image_private_reply";
        val["result"] = "offline";
        thread_write_data(bev, val);
        return;
    }

    // 好友在线，使用分块发送图片消息
    Json::Value extra_fields;
    extra_fields["fromfriend"] = Json::Value::UInt64(userUid);
    thread_send_chunked(friendBev, fileId, fileData, "image_private", extra_fields);

    // 回复发送者成功
    val.clear();
    val["cmd"] = "image_private_reply";
    val["result"] = "success";
    if (v.isMember("request_id"))
        val["request_id"] = v["request_id"];
    thread_write_data(bev, val);
}

void ChatThread::thread_image_group_chat(struct bufferevent *bev, const Json::Value &v)
{
    uint64_t gid = v["gid"].asUInt64();
    uint64_t uid = v["uid"].asUInt64();
    std::string fileId = v["file_id"].asString();
    std::string fileData = v["file_data"].asString();

    // 保存图片到服务器
    std::string filePath = thread_save_file(fileId, fileData);

    if (filePath.empty()) {
        // 保存失败，发送错误消息
        Json::Value errorVal;
        errorVal["cmd"] = "image_group_reply";
        errorVal["result"] = "save_failed";
        if (v.isMember("request_id"))
            errorVal["request_id"] = v["request_id"];
        thread_write_data(bev, errorVal);
        return;
    }

    // 获取发送者用户名
    if (!db->database_connect())
    {
        LOG_ERROR("database_connect");
        return;
    }

    std::string username = db->database_get_username_by_uid(uid);
    db->database_disconnect();

    if (username.empty())
    {
        LOG_ERROR("username not found for uid");
        return;
    }

    // 获取群组名称
    if (!db->database_connect())
    {
        LOG_ERROR("database_connect");
        return;
    }

    std::string groupname = db->database_get_groupname_by_gid(gid);
    db->database_disconnect();

    // 获取成员列表并发送图片消息
    std::list<std::string> member = info->list_get_list_by_gid(gid);
    for (auto it = member.begin(); it != member.end(); it++)
    {
        // 不发送给自己
        if (*it == std::to_string(uid))
            continue;

        struct bufferevent *memberBev = info->list_friend_online(*it);
        if (memberBev == NULL)
            continue;

        Json::Value val;
        val["cmd"] = "image_group";
        val["gid"] = Json::Value::UInt64(gid);
        val["groupname"] = groupname;
        val["from_uid"] = Json::Value::UInt64(uid);
        val["from_username"] = username;
        val["file_id"] = fileId;
        val["file_data"] = fileData;  // 转发Base64编码的图片数据
        thread_write_data(memberBev, val);
    }

    // 回复发送者成功
    Json::Value replyVal;
    replyVal["cmd"] = "image_group_reply";
    replyVal["result"] = "success";
    if (v.isMember("request_id"))
        replyVal["request_id"] = v["request_id"];
    thread_write_data(bev, replyVal);
}