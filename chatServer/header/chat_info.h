#ifndef CHAT_INFO_H
#define CHAT_INFO_H
#include <list>
#include <map>
#include <string>
#include <mutex>
#include <iostream>
#include <jsoncpp/json/json.h>
/*用户类*/
class User
{
public:
    // /**
    //  * @brief 重载==运算符,当name相等时两个User相等
    //  * @param other 另一个User
    //  * @return name相等返回true 不相等返回false
    // */
    // bool operator==(const User& other);

    // 用户ID
    uint64_t uid;
    // 账号(用户名)
    std::string name;
    // 客户端对应的事件
    struct bufferevent *bufevent;
};

/*服务器链表类*/
class ChatInfo
{

public:
    ChatInfo();
    ~ChatInfo();
    /**
     * @brief 更新group_info中群的信息
     * @param g 指的是存放群成员的字符串数组
     * @param size 指的是g中元素个数
     */
    void list_update_group(std::string *g, int size);

    /**
     * @brief 打印group_info的信息
     */
    void list_print_group();

    /**
     * @brief 更新在线用户链表
     * @param bev 当前用户所对应的事件
     * @param v 当前用户个人信息
     * @return 成功返回true 失败返回false
     */
    bool list_update_list(struct bufferevent *bev, Json::Value &v);

    /**
     * @brief 从在线的人中查找user，返回user对应的事件
     * @param user 用户名
     * @return 查找成功返回对应的事件，失败返回NULL
     */
    struct bufferevent *list_friend_online(const std::string &user);

    /**
     * @brief 判断群是否已存在
     * @param groupname 群名称
     * @return 存在返回true 不存在返回false
     */
    bool list_group_is_exist(const std::string &groupname);

    /**
     * @brief 添加新群到group_info中
     * @param groupname 群名称
     * @param owner 群主
     * @note 默认群只有群主一人
    */
    void list_add_new_group(const std::string &groupname,const std::string& owner);

    /**
     * @brief 判断用户是否已经存在于群中
     * @param groupname 群名称
     * @param username 用户名称
     * @note 使用此函数之前请确保群是已经存在的
    */
    bool list_member_is_group(const std::string& groupname,const std::string& username);

    /**
     * @brief 将新群成员user加入到group_info中，更新group_info中的群信息
     * @param groupname 群名称
     * @param username 用户名称
    */
    void list_update_group_member(const std::string &groupname,const std::string &username);

    /**
     * @brief 获取群的成员列表
     * @param groupname 群名称
     * @return 返回成员列表
    */
    std::list<std::string>&list_get_list(const std::string& groupname);  

    /**
     * @brief 删除链表中用户结点
     * @param username 要删除的用户名
    */
    void list_delete_user(const std::string& username);

private:
    // 用于存放已登录的用户信息的链表
    std::list<User> *online_user;
    // 用于存放用户的群的信息
    std::map<std::string, std::list<std::string>> *group_info;
    // 访问在线用户的锁
    std::mutex list_mutex;
    // 访问群信息的锁
    std::mutex map_mutex;

};
#endif