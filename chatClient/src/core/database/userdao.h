#ifndef USERDAO_H
#define USERDAO_H

#include "core/pagedata.h"
#include <QList>

class UserDao
{
public:
    static UserDao& instance();

    bool insertUser(const UserBaseInfoData& user);
    bool updateUser(const UserBaseInfoData& user);
    bool insertOrUpdateUser(const UserBaseInfoData& user);
    bool deleteUser(qint32 ssid);

    UserBaseInfoData getUserBySSID(qint32 ssid);
    QList<UserBaseInfoData> getAllUsers();
    bool userExists(qint32 ssid);

private:
    UserDao() = default;
    ~UserDao() = default;
    UserDao(const UserDao&) = delete;
    UserDao& operator=(const UserDao&) = delete;
};

#endif // USERDAO_H
