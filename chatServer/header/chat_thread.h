#ifndef CHATTHREAD_H
#define CHATTHREAD_H
#include "chat_info.h"
#include "data_base.h"
#include <thread>
#include <event.h>
#include <cstring>
#include <memory>
#include <jsoncpp/json/json.h>
#include "event_utils.h"
class ChatThread
{
public:
    /**
     * @brief 构造函数，创建事件集合，创建新线程，并让其开始工作
     * @details 调用了worker函数
    */
    ChatThread();
    ~ChatThread();

    /**
     * @brief 给info和db初始化
     * @param info 链表指针，指向服务器的链表
     * @param db 数据库指针，指向服务器的数据库
     */
    void start(std::shared_ptr<ChatInfo> &info, std::shared_ptr<DataBase> &db);

    /**
     * @brief 获取线程号
     * @return 返回线程id
     */
    std::thread::id thread_get_id();

    /**
     * @brief 获取事件集合
     * @return 返回事件集合
    */
    struct event_base *thread_get_base();

    /**
     * @brief 处理读事件的回调函数
     * @param bev 对应事件 
     * @param ctx 传给回调函数的参数
     * @details 处理注册、登录、添加好友、私聊等事件
    */
    static void thread_read_cb(struct bufferevent *bev, void *ctx);

    /**
     * @brief 处理异常事件的回调函数
     * @param bev 对应事件
     * @param events 异常事件
     * @param ctx 传给回调函数的参数
    */
    static void thread_event_cb(struct bufferevent *bev, short events, void *ctx);

    /**
     * @brief 写入数据，供客户端接收
     * @param bev 对应事件
     * @param v 要写入的数据
    */
    void thread_write_data(struct bufferevent* bev,const Json::Value& v);

private:
    
    /**
     * @brief 用于worker函数内部，是工作的具体行为
     */
    void run();

    /**
     * @brief 工作函数，用于线程池工作
     * @param 要操作的线程
     */
    static void worker(ChatThread* t);
    
    /**
     * @brief 定时器的回调函数
     */
    static void timeout_cb(evutil_socket_t fd, short event, void *arg);
    
    /**
     * @brief 读取数据的函数,解决TCP分包问题
     * @return 成功读取返回true 否则返回false
     * @note 在thread_readcb中被调用
    */
    bool thread_read_data(struct bufferevent* bev,char *s,size_t buffer_size);

    /**
     * @brief 根据"|"来解析字符串，并将解析的结果放入s中
     * @param f 待解析的字符串
     * @param s 字符串数组，存放解析结果
     * @param buffer_size 缓冲区大小
     * @return 返回解析出的字符串个数
    */
    int thread_parse_string(std::string &f,std::string *s);
    
    /**
     * @brief 处理客户端登录事件
     * @param bev 对应事件
     * @param v 接收到的Json数据
    */
    void thread_login(struct bufferevent* bev,Json::Value& v);
    
    /**
     * @brief 处理客户端注册事件
     * @param bev 对应事件
     * @param v 接收到的Json数据
    */
    void thread_register(struct bufferevent* bev,Json::Value& v);

    /**
     * @brief 处理添加好友事件
     * @param bev 发出请求的客户端事件
     * @param v 客户端信息
    */
    void thread_add_friend(struct bufferevent* bev,const Json::Value& v);

    /**
     * @brief 处理私聊事件
     * @param bev 发出请求的客户端事件
     * @param v 私聊信息
    */
    void thread_private_chat(struct bufferevent* bev,const Json::Value& v);

    /**
     * @brief 处理创建群聊事件
     * @param bev 发出请求的客户端事件
     * @param v 群信息
    */
    void thread_create_group(struct bufferevent* bev,const Json::Value& v);

    /**
     * @brief 处理添加群事件
     * @param bev 发出请求的客户端事件
     * @param v 请求加入的群的信息
    */
    void thread_join_group(struct bufferevent* bev,const Json::Value& v);

    /**
     * @brief 处理群聊事件
     * @param bev 发出请求的客户端事件
     * @param v 群聊消息
    */
    void thread_group_chat(struct bufferevent* bev,const Json::Value& v);

    /**
     * @brief 处理文件传输事件
     * @param bev 发出请求的客户端事件
     * @param v 文件消息
    */
    void thread_transer_file(struct bufferevent* bev,const Json::Value& v);

    /**
     * @brief 处理客户端下线事件
     * @param bev 发出请求的客户端事件
     * @param v 客户端信息
    */
    void thread_client_offline(struct bufferevent* bev,const Json::Value& v);

    /**
     * @brief 处理客户端通过uid查询用户信息事件
     * @param bev 发出请求的客户端事件
     * @param v 要查询的用户信息
    */
    void thread_query_user_by_uid(struct bufferevent* bev,const Json::Value& v);

    /**
     * @brief 处理客户端添加好友时模糊查询事件
     * @param bev 发出请求的客户端事件
     * @param v 要查询的用户信息
    */
    void thread_query_fuzzy_search(struct bufferevent* bev,const Json::Value& v);
    // 事件集合
    std::unique_ptr<struct event_base,EventBaseDeleter>base;
    // 线程
    std::unique_ptr<std::thread>_thread;
    // 线程号
    std::thread::id _id;
    // 数据库对象
    std::shared_ptr<DataBase>db;
    // 数据结构对象
    std::shared_ptr<ChatInfo>info;
};
#endif