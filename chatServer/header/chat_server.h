#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H
#include<event.h>
#include<event2/listener.h>
#include<iostream>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<memory>
#include<vector>
#include"data_base.h"
#include"chat_info.h"
#include"chat_thread.h"
#include "event_utils.h"
#define IP "192.168.200.130"
#define PORT 8888
//最大群组个数
#define GROUP_MAX_SZ 1024
//线程池中最大线程数量
#define POOL_MAX_THREAD_SZ 3

class ChatServer
{
public:
    /**
     * @brief 构造函数
     * @details 初始化事件集合、数据库和数据结构对象，更新群消息列表
    */
    ChatServer();
    ~ChatServer();

    /**
     * @brief 启动服务器，开始监听指定的IP和端口。
     * @param ip 要监听的IP地址 (例如 "0.0.0.0" 表示监听所有网络接口)。
     * @param port 要监听的端口号。
     * @note 必须先初始化并运行一个event_base循环。
     */
    void listen(const char* ip,int port);

    /**
     * @brief 给客户端分配一个socket事件,用线程池中的一个线程来接待它 
     * @param fd 代表文件描述符
     * @details 接待客户端的线程由cur_thread决定
    */
    void server_alloc_event(int fd);
    
private:
    //事件集合对象
    std::unique_ptr<struct event_base,EventBaseDeleter>base;
    //数据库对象
    std::shared_ptr<DataBase>db;
    //数据结构对象
    std::shared_ptr<ChatInfo>info;
    //ID生成器对象
    std::shared_ptr<SnowflakeIDGenerator>id_generator;
    //线程池对象
    std::vector<std::unique_ptr<ChatThread>>pool;
    //当前线程数量
    int thread_num;
    //当前的线程
    int cur_thread;

    /**
     * @brief 监听器的回调函数，在新连接被建立时调用
     * @param 参考官方文档
    */
    static void listener_cb(struct evconnlistener* listener,evutil_socket_t sock,
    struct sockaddr* addr,int len,void* ptr);

    /**
     * @brief 初始化群消息，更新群消息列表
     * @details 把群信息从数据库读出来，放入map
    */
    void server_update_group_info();
    

};
#endif