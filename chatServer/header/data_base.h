#ifndef DATABASE_H
#define DATABASE_H
#include<mysql/mysql.h>
#include<mutex>
#include<stdio.h>
#include<iostream>
#include<jsoncpp/json/json.h>
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
    int data_base_get_group_info(std::string* g);

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

private:
    MYSQL* mysql;
    //数据库的锁 后面可以优化成读写锁
    std::mutex mutex_;
};

#endif