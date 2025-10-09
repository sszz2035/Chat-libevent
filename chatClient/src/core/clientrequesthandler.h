#ifndef CLIENTREQUESTHANDLER_H
#define CLIENTREQUESTHANDLER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <functional>
#include <mutex>
#include<vector>
#include "utils/net-work/clientconserver.h"
#include"page-view/landpage.h"
#include"page-view/addpage.h"
#include<QJsonObject>
class ClientRequestHandler:public QObject
{
    Q_OBJECT

public:
    static ClientRequestHandler* getInstance();
    void destroyInstance();
    
    // 异步查询用户信息
    using QueryCallback = std::function<void(const QJsonObject&)>;
    void queryUserInfoByUid(const qint32 &uid, QueryCallback callback);

    void queryGroupInfoByGid(const qint32& gid,QueryCallback callback);

    // 批量查询用户信息
    void queryUserInfoBatch(const std::vector<qint32>&uids,QueryCallback callback);

signals:
    void userInfoReceived(const QJsonObject& userInfo);
    //模糊搜索请求
    void queryFuzzySearchRequest(const QString& content,bool isGroup,QueryCallback callback);
    //好友申请结果回复
    void addFriendResponse(const QJsonObject& obj);
    void addGroupResponse(const QJsonObject& obj);
    //新用户加入群回复
    void newMemberJoinResponse(const QJsonObject& obj);
public slots:
    void client_reply_info();
    void queryFuzzySearchRequsetHandler(const QString& content,bool isGroup,QueryCallback callback);
    void addFriendRequestHandler(const qint32& ssid,bool isGroup);
    void sendMessageContentHandler(const QList<MessageContentData>& data);
    void createGroupRequestHandler(const QList<int>& data);

private:
    explicit ClientRequestHandler();
    ClientRequestHandler(const ClientRequestHandler&);
    ~ClientRequestHandler();
    ClientRequestHandler& operator=(const ClientRequestHandler&);

    static ClientRequestHandler* instance;
    
    // 请求管理
    QMap<QString, QueryCallback> m_pendingRequests;
    QTimer m_timeoutTimer;
    std::mutex m_requestsMutex;  // 保护m_pendingRequests的互斥锁
    QString generateRequestId();
    void cleanupTimeoutRequests();
};

#endif // CLIENTREQUESTHANDLER_H
