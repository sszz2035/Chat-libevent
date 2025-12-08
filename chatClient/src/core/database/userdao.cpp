#include "userdao.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

UserDao& UserDao::instance()
{
    static UserDao instance;
    return instance;
}

bool UserDao::insertUser(const UserBaseInfoData& user)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO user_info (ssid, username, avatar_path) VALUES (?, ?, ?)");
    query.addBindValue(user.ssid);
    query.addBindValue(user.username);
    query.addBindValue(user.avatarPath);

    if (!query.exec()) {
        qWarning() << "Failed to insert user:" << query.lastError().text();
        return false;
    }
    return true;
}

bool UserDao::updateUser(const UserBaseInfoData& user)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE user_info SET username = ?, avatar_path = ? WHERE ssid = ?");
    query.addBindValue(user.username);
    query.addBindValue(user.avatarPath);
    query.addBindValue(user.ssid);

    if (!query.exec()) {
        qWarning() << "Failed to update user:" << query.lastError().text();
        return false;
    }
    return true;
}

bool UserDao::insertOrUpdateUser(const UserBaseInfoData& user)
{
    if (userExists(user.ssid)) {
        return updateUser(user);
    } else {
        return insertUser(user);
    }
}

bool UserDao::deleteUser(qint32 ssid)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM user_info WHERE ssid = ?");
    query.addBindValue(ssid);

    if (!query.exec()) {
        qWarning() << "Failed to delete user:" << query.lastError().text();
        return false;
    }
    return true;
}

UserBaseInfoData UserDao::getUserBySSID(qint32 ssid)
{
    UserBaseInfoData user;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT ssid, username, avatar_path FROM user_info WHERE ssid = ?");
    query.addBindValue(ssid);

    if (query.exec() && query.next()) {
        user.ssid = query.value(0).toInt();
        user.username = query.value(1).toString();
        user.avatarPath = query.value(2).toString();
    }
    return user;
}

QList<UserBaseInfoData> UserDao::getAllUsers()
{
    QList<UserBaseInfoData> users;
    QSqlQuery query(DatabaseManager::instance().database());

    if (query.exec("SELECT ssid, username, avatar_path FROM user_info")) {
        while (query.next()) {
            UserBaseInfoData user;
            user.ssid = query.value(0).toInt();
            user.username = query.value(1).toString();
            user.avatarPath = query.value(2).toString();
            users.append(user);
        }
    }
    return users;
}

bool UserDao::userExists(qint32 ssid)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM user_info WHERE ssid = ?");
    query.addBindValue(ssid);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}
