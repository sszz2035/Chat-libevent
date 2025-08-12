#ifndef DATABASE_H
#define DATABASE_H
#include<mysql/mysql.h>
#include<mutex>
//数据库类
class DataBase
{
public:
    DataBase();
    ~DataBase();

    /**
     * @brief 与数据库建立连接
     * @return true代表成功连接 false代表连接失败
    */  
    bool database_connect();
    
    /**
     * @brief 与数据库断开连接
    */
    void database_disconnect();

    /**
     * @brief 获取数据库中的群信息
     * @param 用于接收查询结果的字符串的数组
     * @return 查询结果的条数
    */  
    int data_base_get_group_info(std::string* g);

private:
    MYSQL* mysql;
    //数据库的锁 后面可以优化成读写锁
    std::mutex mutex_;
};

#endif