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
#include"data_base.h"
#include"chat_info.h"

#define IP "192.168.200.130"
#define PORT 8888

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

private:
    //事件集合对象
    struct event_base* base;
    //数据库对象
    DataBase* db;
    //数据结构对象
    ChatInfo* info;

    /**
     * @brief 监听器的回调函数，在新连接被建立时调用
     * @param 参考官方文档
    */
    static void listener_cb(struct evconnlistener* listener,evutil_socket_t sock,
    struct sockaddr* addr,int len,void* ptr);

    /**
     * @brief 初始化群消息，更新群消息列表
    */
    void server_update_group_info();
};
#endif