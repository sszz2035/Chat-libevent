#ifndef DBCACHEHELPER_H
#define DBCACHEHELPER_H

#include "core/pagedata.h"
#include "userdao.h"
#include "groupdao.h"
#include "friendshipdao.h"
#include <QJsonObject>
#include <functional>

class DbCacheHelper
{
public:
    static DbCacheHelper& instance();

    void getUserInfo(qint32 ssid,
                     std::function<void(const UserBaseInfoData&)> callback,
                     bool forceRefresh = false);

    void getGroupInfo(qint32 gid,
                      std::function<void(const GroupBaseInfoData&, const QList<GroupMemberInfoData>&)> callback,
                      bool forceRefresh = false);

    void saveUserToCache(const UserBaseInfoData& user);

    void saveGroupToCache(const GroupBaseInfoData& group, const QList<GroupMemberInfoData>& members);

    void saveFriendshipsToCache(qint32 userSSID, const QList<FriendshipData>& friendships);

    QList<FriendshipData> loadFriendshipsFromCache(qint32 userSSID);

private:
    DbCacheHelper() = default;
    ~DbCacheHelper() = default;
    DbCacheHelper(const DbCacheHelper&) = delete;
    DbCacheHelper& operator=(const DbCacheHelper&) = delete;
};

#endif // DBCACHEHELPER_H
