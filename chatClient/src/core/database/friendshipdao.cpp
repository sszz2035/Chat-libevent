#include "friendshipdao.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

FriendshipDao& FriendshipDao::instance()
{
    static FriendshipDao instance;
    return instance;
}

bool FriendshipDao::insertFriendship(const FriendshipData& friendship)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO friendship (ssid, friend_ssid, grouping_name, friend_name, status, friend_type) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(friendship.ssid);
    query.addBindValue(friendship.friendSSID);
    query.addBindValue(friendship.groupingName);
    query.addBindValue(friendship.friendName);
    query.addBindValue(friendship.status);
    query.addBindValue(friendship.friendType);

    if (!query.exec()) {
        qWarning() << "Failed to insert friendship:" << query.lastError().text();
        return false;
    }
    return true;
}

bool FriendshipDao::updateFriendship(const FriendshipData& friendship)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE friendship SET grouping_name = ?, friend_name = ?, status = ?, friend_type = ? "
                  "WHERE ssid = ? AND friend_ssid = ?");
    query.addBindValue(friendship.groupingName);
    query.addBindValue(friendship.friendName);
    query.addBindValue(friendship.status);
    query.addBindValue(friendship.friendType);
    query.addBindValue(friendship.ssid);
    query.addBindValue(friendship.friendSSID);

    if (!query.exec()) {
        qWarning() << "Failed to update friendship:" << query.lastError().text();
        return false;
    }
    return true;
}

bool FriendshipDao::deleteFriendship(qint32 ssid, qint32 friendSSID)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM friendship WHERE ssid = ? AND friend_ssid = ?");
    query.addBindValue(ssid);
    query.addBindValue(friendSSID);

    if (!query.exec()) {
        qWarning() << "Failed to delete friendship:" << query.lastError().text();
        return false;
    }
    return true;
}

bool FriendshipDao::deleteAllFriendships(qint32 ssid)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM friendship WHERE ssid = ?");
    query.addBindValue(ssid);

    if (!query.exec()) {
        qWarning() << "Failed to delete all friendships:" << query.lastError().text();
        return false;
    }
    return true;
}

FriendshipData FriendshipDao::getFriendship(qint32 ssid, qint32 friendSSID)
{
    FriendshipData friendship;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT ssid, friend_ssid, grouping_name, friend_name, status, friend_type "
                  "FROM friendship WHERE ssid = ? AND friend_ssid = ?");
    query.addBindValue(ssid);
    query.addBindValue(friendSSID);

    if (query.exec() && query.next()) {
        friendship.ssid = query.value(0).toInt();
        friendship.friendSSID = query.value(1).toInt();
        friendship.groupingName = query.value(2).toString();
        friendship.friendName = query.value(3).toString();
        friendship.status = query.value(4).toString();
        friendship.friendType = query.value(5).toInt();
    }
    return friendship;
}

QList<FriendshipData> FriendshipDao::getAllFriendships(qint32 ssid)
{
    QList<FriendshipData> friendships;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT ssid, friend_ssid, grouping_name, friend_name, status, friend_type "
                  "FROM friendship WHERE ssid = ?");
    query.addBindValue(ssid);

    if (query.exec()) {
        while (query.next()) {
            FriendshipData friendship;
            friendship.ssid = query.value(0).toInt();
            friendship.friendSSID = query.value(1).toInt();
            friendship.groupingName = query.value(2).toString();
            friendship.friendName = query.value(3).toString();
            friendship.status = query.value(4).toString();
            friendship.friendType = query.value(5).toInt();
            friendships.append(friendship);
        }
    }
    return friendships;
}

QList<FriendshipData> FriendshipDao::getFriendshipsByType(qint32 ssid, qint32 friendType)
{
    QList<FriendshipData> friendships;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT ssid, friend_ssid, grouping_name, friend_name, status, friend_type "
                  "FROM friendship WHERE ssid = ? AND friend_type = ?");
    query.addBindValue(ssid);
    query.addBindValue(friendType);

    if (query.exec()) {
        while (query.next()) {
            FriendshipData friendship;
            friendship.ssid = query.value(0).toInt();
            friendship.friendSSID = query.value(1).toInt();
            friendship.groupingName = query.value(2).toString();
            friendship.friendName = query.value(3).toString();
            friendship.status = query.value(4).toString();
            friendship.friendType = query.value(5).toInt();
            friendships.append(friendship);
        }
    }
    return friendships;
}

bool FriendshipDao::friendshipExists(qint32 ssid, qint32 friendSSID)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM friendship WHERE ssid = ? AND friend_ssid = ?");
    query.addBindValue(ssid);
    query.addBindValue(friendSSID);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}
