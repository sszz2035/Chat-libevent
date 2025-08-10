#ifndef CHAT_INFO_H
#define CHAT_INFO_H
#include<list>
#include<map>
#include<string>
#include<mutex>

/*用户类*/
class User
{
public:
    //账号(用户名)
    std::string name;
    //客户端对应的事件
    struct bufferevent* bufevent;
};

/*服务器链表类*/
class ChatInfo
{
private:
    //用于存放已登录的用户信息的链表
    std::list<User>* online_user;
    //用于存放用户的群的信息
    std::map<std::string,std::list<User>>* group_info;
    //访问在线用户的锁
    std::mutex list_mutex;
    //访问群信息的锁
    std::mutex map_mutex;
public:
    ChatInfo();
    ~ChatInfo();
};
#endif