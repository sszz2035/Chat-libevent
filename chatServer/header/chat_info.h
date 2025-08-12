#ifndef CHAT_INFO_H
#define CHAT_INFO_H
#include<list>
#include<map>
#include<string>
#include<mutex>
#include<iostream>

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
    std::map<std::string,std::list<std::string>>* group_info;
    //访问在线用户的锁
    std::mutex list_mutex;
    //访问群信息的锁
    std::mutex map_mutex;
public:
    ChatInfo();
    ~ChatInfo();
    /**
     * @brief 更新group_info中群的信息
     * @param g 指的是存放群信息的字符串数组
     * @param size 指的是g中元素个数
    */
    void list_update_group(std::string* g,int size);

    /**
     * @brief 打印group_info的信息
    */
   void list_print_group();

};
#endif