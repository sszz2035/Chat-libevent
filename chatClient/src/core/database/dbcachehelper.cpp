#include "dbcachehelper.h"
#include "databasemanager.h"
#include "core/clientrequesthandler.h"
#include <QDebug>
#include <QTimer>

DbCacheHelper& DbCacheHelper::instance()
{
    static DbCacheHelper instance;
    return instance;
}

void DbCacheHelper::getUserInfo(qint32 ssid,
                                std::function<void(const UserBaseInfoData&)> callback,
                                bool forceRefresh)
{
    // 检查数据库是否已打开
    if (!DatabaseManager::instance().isOpen()) {
        qWarning() << "Database not open, fetching from server directly";
        forceRefresh = true;
    }

    // 1. 先查本地缓存
    if (!forceRefresh) {
        try {
            if (UserDao::instance().userExists(ssid)) {
                UserBaseInfoData user = UserDao::instance().getUserBySSID(ssid);
                if (!user.username.isEmpty()) {
                    qDebug() << "Cache hit for user:" << ssid;
                    // 使用 QTimer 确保回调异步执行，避免在构造函数中同步调用
                    if (callback) {
                        QTimer::singleShot(0, [callback, user]() {
                            callback(user);
                        });
                    }
                    return;
                }
            }
        } catch (...) {
            qWarning() << "Database query failed for user:" << ssid;
        }
    }

    // 2. 本地未命中，请求网络
    qDebug() << "Cache miss for user:" << ssid << ", fetching from server...";
    ClientRequestHandler::getInstance()->queryUserInfoByUid(ssid, [ssid, callback](const QJsonObject& obj) {
        UserBaseInfoData user;
        user.ssid = obj["uid"].toInt();
        user.username = obj["username"].toString();
        user.avatarPath = ":/include/Image/Cirno.jpg";

        // 3. 存入本地缓存
        if (DatabaseManager::instance().isOpen()) {
            UserDao::instance().insertOrUpdateUser(user);
        }

        // 4. 回调返回
        if (callback) callback(user);
    });
}

void DbCacheHelper::getGroupInfo(qint32 gid,
                                 std::function<void(const GroupBaseInfoData&, const QList<GroupMemberInfoData>&)> callback,
                                 bool forceRefresh)
{
    // 检查数据库是否已打开
    if (!DatabaseManager::instance().isOpen()) {
        qWarning() << "Database not open, fetching from server directly";
        forceRefresh = true;
    }

    // 1. 先查本地缓存
    if (!forceRefresh) {
        try {
            if (GroupDao::instance().groupExists(gid)) {
                GroupBaseInfoData group = GroupDao::instance().getGroupBySSID(gid);
                QList<GroupMemberInfoData> members = GroupDao::instance().getGroupMembers(gid);
                if (!group.groupName.isEmpty()) {
                    qDebug() << "Cache hit for group:" << gid;
                    // 使用 QTimer 确保回调异步执行，而不是同步执行
                    if (callback) {
                        QTimer::singleShot(0, [callback, group, members]() {
                            callback(group, members);
                        });
                    }
                    return;
                }
            }
        } catch (...) {
            qWarning() << "Database query failed for group:" << gid;
        }
    }

    // 2. 本地未命中，请求网络
    qDebug() << "Cache miss for group:" << gid << ", fetching from server...";
    ClientRequestHandler::getInstance()->queryGroupInfoByGid(gid, [gid, callback](const QJsonObject& obj) {
        GroupBaseInfoData group;
        QList<GroupMemberInfoData> members;

        group.ssidGroup = obj["gid"].toInt();
        group.groupName = obj["groupname"].toString();
        group.createSSID = obj["groupowner"].toInt();
        group.avatarPath = ":/include/Image/Cirno.jpg";

        // 解析群成员
        QString memberList = obj["groupmember"].toString();
        QStringList memberIds = memberList.split('|', Qt::SkipEmptyParts);
        for (const QString& memberId : memberIds) {
            GroupMemberInfoData member;
            member.ssidGroup = gid;
            member.ssidMember = memberId.toInt();
            members.append(member);
        }

        // 3. 存入本地缓存
        if (DatabaseManager::instance().isOpen()) {
            GroupDao::instance().insertOrUpdateGroup(group);
            GroupDao::instance().deleteAllGroupMembers(gid);
            for (const auto& member : members) {
                GroupDao::instance().insertGroupMember(member);
            }
        }

        // 4. 回调返回
        if (callback) callback(group, members);
    });
}

void DbCacheHelper::saveUserToCache(const UserBaseInfoData& user)
{
    UserDao::instance().insertOrUpdateUser(user);
}

void DbCacheHelper::saveGroupToCache(const GroupBaseInfoData& group, const QList<GroupMemberInfoData>& members)
{
    GroupDao::instance().insertOrUpdateGroup(group);
    GroupDao::instance().deleteAllGroupMembers(group.ssidGroup);
    for (const auto& member : members) {
        GroupDao::instance().insertGroupMember(member);
    }
}

void DbCacheHelper::saveFriendshipsToCache(qint32 userSSID, const QList<FriendshipData>& friendships)
{
    // 先删除该用户的所有好友关系
    FriendshipDao::instance().deleteAllFriendships(userSSID);

    // 插入新的好友关系
    for (const auto& friendship : friendships) {
        FriendshipDao::instance().insertFriendship(friendship);
    }
}

QList<FriendshipData> DbCacheHelper::loadFriendshipsFromCache(qint32 userSSID)
{
    return FriendshipDao::instance().getAllFriendships(userSSID);
}
