#include "groupdao.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

GroupDao& GroupDao::instance()
{
    static GroupDao instance;
    return instance;
}

// ============ 群组信息操作 ============

bool GroupDao::insertGroup(const GroupBaseInfoData& group)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO group_info (ssid_group, group_name, avatar_path, create_ssid) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(group.ssidGroup);
    query.addBindValue(group.groupName);
    query.addBindValue(group.avatarPath);
    query.addBindValue(group.createSSID);

    if (!query.exec()) {
        qWarning() << "Failed to insert group:" << query.lastError().text();
        return false;
    }
    return true;
}

bool GroupDao::updateGroup(const GroupBaseInfoData& group)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE group_info SET group_name = ?, avatar_path = ?, create_ssid = ? "
                  "WHERE ssid_group = ?");
    query.addBindValue(group.groupName);
    query.addBindValue(group.avatarPath);
    query.addBindValue(group.createSSID);
    query.addBindValue(group.ssidGroup);

    if (!query.exec()) {
        qWarning() << "Failed to update group:" << query.lastError().text();
        return false;
    }
    return true;
}

bool GroupDao::insertOrUpdateGroup(const GroupBaseInfoData& group)
{
    if (groupExists(group.ssidGroup)) {
        return updateGroup(group);
    } else {
        return insertGroup(group);
    }
}

bool GroupDao::deleteGroup(qint32 ssidGroup)
{
    // 先删除群成员
    deleteAllGroupMembers(ssidGroup);

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM group_info WHERE ssid_group = ?");
    query.addBindValue(ssidGroup);

    if (!query.exec()) {
        qWarning() << "Failed to delete group:" << query.lastError().text();
        return false;
    }
    return true;
}

GroupBaseInfoData GroupDao::getGroupBySSID(qint32 ssidGroup)
{
    GroupBaseInfoData group;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT ssid_group, group_name, avatar_path, create_ssid "
                  "FROM group_info WHERE ssid_group = ?");
    query.addBindValue(ssidGroup);

    if (query.exec() && query.next()) {
        group.ssidGroup = query.value(0).toInt();
        group.groupName = query.value(1).toString();
        group.avatarPath = query.value(2).toString();
        group.createSSID = query.value(3).toInt();
    }
    return group;
}

QList<GroupBaseInfoData> GroupDao::getAllGroups()
{
    QList<GroupBaseInfoData> groups;
    QSqlQuery query(DatabaseManager::instance().database());

    if (query.exec("SELECT ssid_group, group_name, avatar_path, create_ssid FROM group_info")) {
        while (query.next()) {
            GroupBaseInfoData group;
            group.ssidGroup = query.value(0).toInt();
            group.groupName = query.value(1).toString();
            group.avatarPath = query.value(2).toString();
            group.createSSID = query.value(3).toInt();
            groups.append(group);
        }
    }
    return groups;
}

QList<GroupBaseInfoData> GroupDao::getGroupsByCreator(qint32 createSSID)
{
    QList<GroupBaseInfoData> groups;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT ssid_group, group_name, avatar_path, create_ssid "
                  "FROM group_info WHERE create_ssid = ?");
    query.addBindValue(createSSID);

    if (query.exec()) {
        while (query.next()) {
            GroupBaseInfoData group;
            group.ssidGroup = query.value(0).toInt();
            group.groupName = query.value(1).toString();
            group.avatarPath = query.value(2).toString();
            group.createSSID = query.value(3).toInt();
            groups.append(group);
        }
    }
    return groups;
}

bool GroupDao::groupExists(qint32 ssidGroup)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM group_info WHERE ssid_group = ?");
    query.addBindValue(ssidGroup);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

// ============ 群成员操作 ============

bool GroupDao::insertGroupMember(const GroupMemberInfoData& member)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO group_member (ssid_group, ssid_member) VALUES (?, ?)");
    query.addBindValue(member.ssidGroup);
    query.addBindValue(member.ssidMember);

    if (!query.exec()) {
        qWarning() << "Failed to insert group member:" << query.lastError().text();
        return false;
    }
    return true;
}

bool GroupDao::deleteGroupMember(qint32 ssidGroup, qint32 ssidMember)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM group_member WHERE ssid_group = ? AND ssid_member = ?");
    query.addBindValue(ssidGroup);
    query.addBindValue(ssidMember);

    if (!query.exec()) {
        qWarning() << "Failed to delete group member:" << query.lastError().text();
        return false;
    }
    return true;
}

bool GroupDao::deleteAllGroupMembers(qint32 ssidGroup)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM group_member WHERE ssid_group = ?");
    query.addBindValue(ssidGroup);

    if (!query.exec()) {
        qWarning() << "Failed to delete all group members:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<GroupMemberInfoData> GroupDao::getGroupMembers(qint32 ssidGroup)
{
    QList<GroupMemberInfoData> members;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT ssid_group, ssid_member FROM group_member WHERE ssid_group = ?");
    query.addBindValue(ssidGroup);

    if (query.exec()) {
        while (query.next()) {
            GroupMemberInfoData member;
            member.ssidGroup = query.value(0).toInt();
            member.ssidMember = query.value(1).toInt();
            members.append(member);
        }
    }
    return members;
}

QList<qint32> GroupDao::getGroupMemberSSIDs(qint32 ssidGroup)
{
    QList<qint32> ssids;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT ssid_member FROM group_member WHERE ssid_group = ?");
    query.addBindValue(ssidGroup);

    if (query.exec()) {
        while (query.next()) {
            ssids.append(query.value(0).toInt());
        }
    }
    return ssids;
}

bool GroupDao::isMemberOfGroup(qint32 ssidGroup, qint32 ssidMember)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM group_member WHERE ssid_group = ? AND ssid_member = ?");
    query.addBindValue(ssidGroup);
    query.addBindValue(ssidMember);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}
