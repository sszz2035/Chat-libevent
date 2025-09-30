#ifndef DATABASE_H
#define DATABASE_H
#include <mysql/mysql.h>
#include <mutex>
#include <stdio.h>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <memory>
#include <shared_mutex>
#include "snowflake_id_generator.h"

// 为MYSQL* 指针定制的删除器
struct MysqlDeleter
{
    void operator()(MYSQL *mysql) const
    {
        if (mysql)
        {
            mysql_close(mysql);
        }
    }
};

// 数据库类
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
     * @brief 初始化数据库,创建chat_group和chat_user表
     * @return 成功返回true 失败返回false
     */
    bool database_init_table();

    /**
     * @brief 获取数据库中的群信息
     * @param 用于接收查询结果的字符串的数组
     * @note g数组中的字符串格式形如:groupname|membername1|membername2......
     * @return 查询结果的条数
     */
    int database_get_group_info(std::string *g);

    /**
     * @brief 判断用户是否存在于数据库中
     * @param usr 表示用户名
     * @return 存在返回true 不存在返回false
     */
    bool database_user_is_exist(std::string usr);

    /**
     * @brief 判断用户是否存在于数据库中(通过UID)
     * @param uid 表示用户ID
     * @return 存在返回true 不存在返回false
     */
    bool database_user_is_exist_by_uid(uint64_t uid);

    /**
     * @brief 通过UID获取用户名
     * @param uid 用户ID
     * @return 用户名，如果不存在返回空字符串
     */
    std::string database_get_username_by_uid(uint64_t uid);

    /**
     * @brief 将用户添加到数据库中
     * @param v 用户信息，包含username,password
     * @param uid 用户ID（如果为0则自动生成）
     * @note 对chat_user数据库进行操作
     */
    void database_insert_user_info(Json::Value &v, uint64_t uid = 0);

    /**
     * @brief 判断密码是否正确(通过用户名)
     * @param v 接收到的Json数据
     * @note 对chat_user数据库进行操作
     */
    bool database_password_correct(Json::Value &v);

    /**
     * @brief 判断密码是否正确(通过UID)
     * @param uid 用户ID
     * @param password 密码
     * @return 密码正确返回true，错误返回false
     */
    bool database_password_correct_by_uid(uint64_t uid, const std::string &password);

    /**
     * @brief 获取用户的好友列表与群列表(通过用户名)
     * @param v 用户个人信息,包含username,password
     * @param friList 接收用户好友列表的参数
     * @param groList 接收用户群组列表的参数
     * @return 成功返回true 失败返回false
     */
    bool database_get_friend_group(const Json::Value &v, std::string &friList, std::string &groList);

    /**
     * @brief 获取用户的好友列表与群列表(通过UID)
     * @param uid 用户ID
     * @param friList 接收用户好友列表的参数
     * @param groList 接收用户群组列表的参数
     * @return 成功返回true 失败返回false
     */
    bool database_get_friend_group_by_uid(uint64_t uid, std::string &friList, std::string &groList);

    /**
     * @brief 用于处理添加好友的更新好友列表操作
     * @param u 要更新好友列表的用户
     * @param f 用户的新好友
     * @note 修改chat_user表
     */
    void database_update_friendlist(const std::string &u, const std::string &f);

    /**
     * @brief 用于处理更新群列表变更的操作
     * @param u 要更新群列表的用户
     * @param gid 群id
     */
    void database_update_grouplist(const std::string &u, const std::string &gid);

    /**
     * @brief 添加好友成功后对数据库进行处理的函数\
     * @param v 添加好友请求信息
     */
    void database_add_friend(const Json::Value &v);

    /**
     * @brief 将新群添加到数据库中
     * @param groupname 群名称
     * @param owner 群主名称
     * @return 返回群的gid,失败返回0
     */
    uint64_t database_add_new_group(const std::string &groupname, const std::string &owner);

    /**
     * @brief 将新成员添加到群中
     * @param groupname 群名称
     * @param username 用户名称
     */
    void database_update_group_member(const std::string &groupname, const std::string &username);

    /**
     * @brief 通过群名称获取群ID
     * @param groupname 群名称
     * @return 群ID，如果不存在返回0
     * @note 此函数已弃用
     */
    uint64_t database_get_gid_by_groupname(const std::string &groupname);

    /**
     * @brief 通过群ID获取群名称
     * @param gid 群ID
     * @return 群名称，如果不存在返回空字符串
     */
    std::string database_get_groupname_by_gid(uint64_t gid);

    /**
     * @brief 通过群ID将新成员添加到群中
     * @param gid 群ID
     * @param uid 用户ID
     */
    void database_update_group_member_by_gid(uint64_t gid, const std::string &uid);

    /**
     * @brief 更新用户的群组列表
     * @param uid 用户ID
     * @param gid 群组ID
     */
    void database_update_grouplist_by_uid(uint64_t uid, const std::string &gid);

public:
    /**
     * @brief [辅助函数] 用于执行查询命令，并返回查询结果
     * @param sql 查询语句
     * @param row 接收查询结果
     * @note 返回的是一行的结果
     * @return 成功返回true 失败返回false
     */
    bool exec_query_and_fetch_row(const char *sql, MYSQL_ROW &row);

    /**
     * @brief [辅助函数] 用于执行查询命令，并返回多行查询结果
     * @param sql 查询语句
     * @param rows 接收查询结果，每行是一个vector<string>
     * @note 返回多行结果，处理NULL值
     * @return 查询到的行数，出错返回-1
     */
    int exec_query_and_fetch_rows(const char *sql, std::vector<std::vector<std::string>> &rows);

private:
    /**
     * @brief [辅助函数] 用于执行不返回结果的sql语句(UPDATE,INSERT,DELETE)
     * @param sql 查询语句
     * @return 成功返回true 失败返回false
     */
    bool exec_update(const char *sql);
    // 数据库指针
    std::unique_ptr<MYSQL, MysqlDeleter> mysql;
    // 数据库的锁(读写锁)
    std::shared_mutex mutex_;
    // ID生成器
    std::shared_ptr<SnowflakeIDGenerator> id_generator_;
};

#endif