#ifndef GROUPDAO_H
#define GROUPDAO_H

#include "core/pagedata.h"
#include <QList>

class GroupDao
{
public:
    static GroupDao& instance();

    // 群组信息操作
    bool insertGroup(const GroupBaseInfoData& group);
    bool updateGroup(const GroupBaseInfoData& group);
    bool insertOrUpdateGroup(const GroupBaseInfoData& group);
    bool deleteGroup(qint32 ssidGroup);

    GroupBaseInfoData getGroupBySSID(qint32 ssidGroup);
    QList<GroupBaseInfoData> getAllGroups();
    QList<GroupBaseInfoData> getGroupsByCreator(qint32 createSSID);
    bool groupExists(qint32 ssidGroup);

    // 群成员操作
    bool insertGroupMember(const GroupMemberInfoData& member);
    bool deleteGroupMember(qint32 ssidGroup, qint32 ssidMember);
    bool deleteAllGroupMembers(qint32 ssidGroup);

    QList<GroupMemberInfoData> getGroupMembers(qint32 ssidGroup);
    QList<qint32> getGroupMemberSSIDs(qint32 ssidGroup);
    bool isMemberOfGroup(qint32 ssidGroup, qint32 ssidMember);

private:
    GroupDao() = default;
    ~GroupDao() = default;
    GroupDao(const GroupDao&) = delete;
    GroupDao& operator=(const GroupDao&) = delete;
};

#endif // GROUPDAO_H
