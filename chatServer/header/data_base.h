#ifndef DATABASE_H
#define DATABASE_H
#include<mysql/mysql.h>
#include<mutex>
#include<stdio.h>
#include<iostream>
#include<jsoncpp/json/json.h>
#include <shared_mutex>
//数据库类
class DataBase
{
public:
    DataBase();
    ~DataBase();

    /**
     * @brief 与数据库建立连接
     * @return 成功返回true 失败返回false
     * @note 后续可考虑用连接池优化
    */  
    bool database_connect();
    
    /**
     * @brief 与数据库断开连接
    */
    void database_disconnect();

    /**
     * @brief 初始化数据库
     * @return 成功返回true 失败返回false
    */
    bool database_init_table();

    /**
     * @brief 获取数据库中的群信息
     * @param 用于接收查询结果的字符串的数组
     * @note g数组中的字符串格式形如:groupname|membername1|membername2......
     * @return 查询结果的条数
    */  
    int database_get_group_info(std::string* g);

    /**
     * @brief 判断用户是否存在于数据库中
     * @param usr 表示用户名
     * @return 存在返回true 不存在返回false
    */
    bool database_user_is_exist(std::string usr);

    /**
     * @brief 将用户添加到数据库中
     * @note 对chat_user数据库进行操作
    */
   void database_insert_user_info(Json::Value& v);

    /**
     * @brief 判断密码是否正确
     * @param v 接收到的Json数据
     * @note 对chat_user数据库进行操作
    */
   bool database_password_correct(Json::Value& v);

   /**
    * @brief 获取用户的好友列表与群列表
    * @param v 用户个人信息,包含username,password 
    * @param friList 接收用户好友列表的参数
    * @param groList 接收用户群组列表的参数
    * @return 成功返回true 失败返回false
   */
   bool database_get_friend_group(Json::Value& v,std::string& friList,std::string& groList);
private:
    MYSQL* mysql;
    //数据库的锁(读写锁)
    std::shared_mutex mutex_;
};

#endif