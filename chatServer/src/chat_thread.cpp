#include "chat_thread.h"
#include "log.h"
ChatThread::ChatThread():base(event_base_new()),
_thread(std::make_unique<std::thread>(worker,this)),
_id(_thread->get_id())
{
}

ChatThread::~ChatThread()
{

}

void ChatThread::start(std::shared_ptr<ChatInfo> &info,std::shared_ptr<DataBase> &db)
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
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if (!t->thread_read_data(bev, buf))
    {
        LOG_PERROR("thread_read_data");
        return;
    }
    std::cout << "--thread " << t->thread_get_id() << " receive data " << buf << std::endl;

    // 解析json数据
    Json::Reader reader; // json 解析对象
    Json::Value val;     // json存储的容器 类似于map

    if (!reader.parse(buf, val))
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
    //处理私聊事件
    else if(val["cmd"]=="private")
    {
        t->thread_private_chat(bev,val);
    }
    //处理添加群组事件
    else if(val["cmd"]=="creategroup")
    {
        t->thread_create_group(bev,val);
    }
    //处理加入群聊事件
    else if(val["cmd"]=="joingroup")
    {
        t->thread_join_group(bev,val);
    }
    //处理群聊事件
    else if(val["cmd"]=="groupchat")
    {   
        t->thread_group_chat(bev,val);
    }
    //处理文件传输事件
    else if(val["cmd"]=="file")
    {
        t->thread_transer_file(bev,val);
    }
    //处理客户端下线事件
    else if(val["cmd"]=="offline")
    {
        t->thread_client_offline(bev,val);
    }
}

void ChatThread::thread_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    //客户端连接已关闭
    if(events& BEV_EVENT_EOF)
    {
        std::cout<<"[disconnect] client offline"<<std::endl;
        bufferevent_free(bev);
    }
    else
    {
        std::cout<<"Unknown Error"<<std::endl;
    }
}

bool ChatThread::thread_read_data(struct bufferevent *bev, char *s)
{
    // 要读取的长度
    int sz = 0;
    // 先读取长度，放入sz中
    if (bufferevent_read(bev, &sz, 4) != 4)
    {
        LOG_PERROR("bufferevent_read");
        return false;
    }
    size_t count = 0;
    char buf[1024] = {0};
    while (1)
    {
        // count加上读取到的字符个数
        count += bufferevent_read(bev, buf, 1024);
        strcat(s, buf);
        memset(buf, 0, sizeof(buf));
        // 读取到的字符个数>=要读取的字符个数
        if (count >= sz)
            break;
    }
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
    // 判断用户是否已存在

    // 用户已存在
    if (db->database_user_is_exist(v["username"].asString()))
    {
        Json::Value val;
        val["cmd"] = "register_reply";
        val["result"] = "user_exist";
        // 发送回应给客户端
        thread_write_data(bev, val);
    }

    // 用户不存在
    else
    {
        // 将用户添加到数据库中，会自动生成UID
        db->database_insert_user_info(v);
        
        // 获取新生成的UID
        std::string username = v["username"].asString();
        char sql[256];
        sprintf(sql, "SELECT uid FROM chat_user WHERE username='%s';", username.c_str());
        MYSQL_ROW row;
        uint64_t new_uid = 0;
        
        // 查询UID（数据库已经连接）
        if (db->exec_query_and_fetch_row(sql, row)) {
            new_uid = std::stoull(row[0]);
        }
        
        // 发送回应给客户端
        Json::Value val;
        val["cmd"] = "register_reply";
        val["result"] = "success";
        val["uid"] = Json::Value::UInt64(new_uid);
        val["username"] = username;
        thread_write_data(bev, val);
    }

    // 断开数据库
    db->database_disconnect();
}

void ChatThread::thread_write_data(struct bufferevent *bev, const Json::Value &v)
{
    // 封装json数据为字符串
    Json::FastWriter writer;
    std::string val = writer.write(v);
    int len = val.size();
    char wbuf[1024] = {0};
    memcpy(wbuf, &len, 4);
    memcpy(wbuf + 4, val.c_str(), len);
    // 将数据写入输出缓冲区中
    bufferevent_write(bev, wbuf, len + 4);
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
    
    if (v.isMember("uid") && v["uid"].isUInt64()) {
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
    if (loginUid > 0) {
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
    
    if (loginUid > 0) {
        get_friend_group_success = db->database_get_friend_group_by_uid(loginUid, friendlist, grouplist);
    }
    if (!get_friend_group_success)
    {
        LOG_PERROR("database_get_friend_group");
        db->database_disconnect();
        return;
    }
    loginName=db->database_get_username_by_uid(loginUid);
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
    std::string friendName = v["friend"].asString();
    // 判断用户是否存在
    // 用户不存在
    if (!db->database_user_is_exist(friendName))
    {
        val["cmd"] = "addfriend_reply";
        val["result"] = "not_exist";
        thread_write_data(bev, val);
        db->database_disconnect();
        return;
    }

    // 判断是否是好友关系
    std::string Friend[1024];
    std::string friendlist, grouplist;
    if (db->database_get_friend_group(v, friendlist, grouplist))
    {
        int num = thread_parse_string(friendlist, Friend);
        for (int i = 0; i < num; i++)
        {
            if (Friend[i] == friendName)
            {
                val["cmd"] = "addfriend_reply";
                val["result"] = "already_friend";
                thread_write_data(bev, val);
                db->database_disconnect();
                return;
            }
        }
    }

    // 修改数据库
    db->database_add_friend(v);
    
    // 回复好友
    val["cmd"]="be_addfriend";
    val["friend"]=v["username"];
    struct bufferevent* b=info->list_friend_online(v["friend"].asString());
    if(b!=NULL) thread_write_data(b,val);

    // 回复客户端
    val.clear();
    val["cmd"]="addfriend_reply";
    val["result"]="success";
    thread_write_data(bev,val);

    db->database_disconnect();
}

void ChatThread::thread_private_chat(struct bufferevent* bev,const Json::Value& v)
{
    std::string username=v["username"].asString();
    std::string friendname=v["tofriend"].asString();
    struct bufferevent* b=info->list_friend_online(friendname);
    Json::Value val;
    //如果好友不在线
    if(b==NULL)
    {
        val["cmd"]="private_reply";
        val["result"]="offline";
        thread_write_data(bev,val);
        return;
    }
    //好友在线
    val["cmd"]="private";
    val["fromfriend"]=username;
    val["text"]=v["text"];
    thread_write_data(b,val);
}

void ChatThread::thread_create_group(struct bufferevent* bev,const Json::Value& v)
{
    std::string groupname=v["groupname"].asString();
    Json::Value val;
    //判断是否已有群
    if(info->list_group_is_exist(groupname))
    {
        val["cmd"]="creategroup_reply";
        val["result"]="exist";
        thread_write_data(bev,val);
        return;
    }

    //在数据库中添加新群
    if(!db->database_connect())
    {
        LOG_ERROR("database_connect");
        return;
    }
    db->database_add_new_group(groupname,v["owner"].asString());
    db->database_disconnect();
    
   //将新群添加到群信息中
    info->list_add_new_group(groupname,v["owner"].asString());

    val["cmd"]="creategroup_reply";
    val["result"]="success";
    thread_write_data(bev,val);
}

void ChatThread::thread_join_group(struct bufferevent* bev,const Json::Value& v)
{
    std::string groupname=v["groupname"].asString();
    std::string username=v["username"].asString();
    Json::Value val;
    //判断群是否存在
    if(!info->list_group_is_exist(groupname))
    {
        val["cmd"]="joingroup_reply";
        val["result"]="not_exist";
        thread_write_data(bev,val);
        return;
    }
    //判断群中是否已有当前成员
    if(info->list_member_is_group(groupname,username))
    {
        val["cmd"]="joingroup_reply";
        val["result"]="already";
        thread_write_data(bev,val);
        return;
    }
    //修改数据库
    db->database_connect();
    db->database_update_group_member(groupname,username);
    db->database_disconnect();

    //更新链表中群信息
    info->list_update_group_member(groupname,username);

    //向群中所有用户发送消息
    std::list<std::string>l=info->list_get_list(groupname);
    std::string member;
    for(auto it=l.begin();it!=l.end();it++)
    {
        if(*it==username)   continue;
        member.append(*it);
        member.append("|");
        struct bufferevent* b=info->list_friend_online(*it);
        if(b==NULL) continue;
        val["cmd"]="new_member_join";
        val["groupname"]=groupname;
        val["username"]=username;
        thread_write_data(b,val);
    }
    //去掉最后的'|'
    member.erase(member.size()-1);
    val.clear();
    //向自己发送消息
    val["cmd"]="joingroup_reply";
    val["result"]="success";
    val["member"]=member;
    thread_write_data(bev,val);
}   

void ChatThread::thread_group_chat(struct bufferevent* bev,const Json::Value& v)
{
    std::string groupname=v["groupname"].asString();
    std::string username=v["username"].asString();
    std::string text=v["text"].asString();
    //获取成员列表
    std::list<std::string>member=info->list_get_list(groupname);
    //将消息转发给群成员
    for(auto it=member.begin();it!=member.end();it++)
    {
        if(*it==username)   continue;
        struct bufferevent* b=info->list_friend_online(*it);
        if(b==NULL) continue;
        Json::Value val;
        val["cmd"]="groupchat_reply";
        val["groupname"]=groupname;
        val["from"]=username;
        val["text"]=text;
        thread_write_data(b,val);
    }
}

void ChatThread::thread_transer_file(struct bufferevent* bev,const Json::Value& v)
{
    std::string username=v["username"].asString();
    std::string friendname=v["friendname"].asString();
    std::string filename=v["filename"].asString();
    std::string filelength=v["filelength"].asString();
    std::string step=v["step"].asString();
    Json::Value val;
    //判断对方是否在线
    struct bufferevent* b=info->list_friend_online(friendname);
    if(b==NULL)
    {
        val["cmd"]="file_reply";
        val["result"]="offline";
        thread_write_data(bev,val);
        return;
    }
    //对step判断
    if(step=="1")
    {
        val["cmd"]="file_name";
        val["filename"]=filename;
        val["filelength"]=filelength;
        val["fromuser"]=username;
    }   
    else if(step=="2")
    {
        std::string text=v["text"].asString();
        val["cmd"]="file_transfer";
        val["text"]=text;
    }
    else if(step=="3")
    {
        val["cmd"]="file_end";
    }   
    thread_write_data(b,val);
}

void ChatThread::thread_client_offline(struct bufferevent* bev,const Json::Value& v)
{
    std::string username=v["username"].asString();
    
    //删除info的online_user中的客户端事件
    info->list_delete_user(username);

    //释放客户端事件
    bufferevent_free(bev);

    //通知所有好友
    std::string friendlist,grouplist;
    db->database_connect();
    //获取好友列表
    db->database_get_friend_group(v,friendlist,grouplist);

    db->database_disconnect();

    std::string Friend[1024];

    if(friendlist.empty())  return;
    //将好友列表解析到Friend数组
    int num=thread_parse_string(friendlist,Friend);
    //通知所有好友
    for(int i=0;i<num;i++)
    {
        struct bufferevent* b=info->list_friend_online(Friend[i]);
        if(b==NULL) continue;
        Json::Value val;
        val["cmd"]="friend_offline";
        val["username"]=username;
        thread_write_data(b,val);
    }
}
