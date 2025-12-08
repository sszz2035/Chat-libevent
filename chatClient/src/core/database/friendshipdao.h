#ifndef FRIENDSHIPDAO_H
#define FRIENDSHIPDAO_H

#include "core/pagedata.h"
#include <QList>

class FriendshipDao
{
public:
    static FriendshipDao& instance();

    bool insertFriendship(const FriendshipData& friendship);
    bool updateFriendship(const FriendshipData& friendship);
    bool deleteFriendship(qint32 ssid, qint32 friendSSID);
    bool deleteAllFriendships(qint32 ssid);

    FriendshipData getFriendship(qint32 ssid, qint32 friendSSID);
    QList<FriendshipData> getAllFriendships(qint32 ssid);
    QList<FriendshipData> getFriendshipsByType(qint32 ssid, qint32 friendType);
    bool friendshipExists(qint32 ssid, qint32 friendSSID);

private:
    FriendshipDao() = default;
    ~FriendshipDao() = default;
    FriendshipDao(const FriendshipDao&) = delete;
    FriendshipDao& operator=(const FriendshipDao&) = delete;
};

#endif // FRIENDSHIPDAO_H
