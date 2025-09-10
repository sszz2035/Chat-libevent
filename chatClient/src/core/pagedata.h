#ifndef PAGEDATA_H
#define PAGEDATA_H
#include<QString>
#include <qobject.h>

struct UserBaseInfoData{
    qint32     ssid;
    QString     username = "";
    QString     avatarPath = "";
    // QString     sex = "";
    // QString     personalSign = "";
    // time_t      birthDate = 0;
    // quint32     thumbUpCount = static_cast<quint32>(-1);
    // quint8      region =  static_cast<quint8>(-1);
    // time_t      createTime = 0;
    UserBaseInfoData() = default;
    UserBaseInfoData(const UserBaseInfoData& other) = default;
};

struct FriendshipData
{
    qint32 ssid;
    qint32 friendSSID;
    QString groupingName;//分组名
    QString friendName;//好友名或群组名
    QString status;
    qint32 friendType;//1好友 2群组
};
// Q_DECLARE_METATYPE(FriendshipData)

struct GroupBaseInfoData{
    qint32         ssidGroup;
    QString        groupName;
    QString        avatarPath;
    qint32         createSSID;
    // QString        profile;
    // QList<QString> admins;
    // time_t         createTime;
};

struct GroupMemberInfoData{
    qint32 ssidGroup;
    qint32 ssidMember;
    // time_t createTime;
};

struct MsgCombineData {
    UserBaseInfoData              userBaseInfo;
    GroupBaseInfoData             groupBaseInfo;//群信息
    QList<GroupMemberInfoData>    groupMemberInfo;//群成员链表
    QString                       content;
    // qint64                       timestamp = 0;
    bool isGroup = false;
};
// Q_DECLARE_METATYPE(MsgCombineDTO)

#endif // PAGEDATA_H
