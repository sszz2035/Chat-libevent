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
#include "core/pagedata.h"  // 添加MessageContentData支持
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
    // 图片处理完成信号（用于线程安全的UI更新）
    void imageProcessed(const MessageContentData& content);
    // 原始数据就绪信号（用于异步处理大数据）
    void rawDataReady(const QByteArray& data);
public slots:
    void client_reply_info(const QByteArray& ba);
    void queryFuzzySearchRequsetHandler(const QString& content,bool isGroup,QueryCallback callback);
    void addFriendRequestHandler(const qint32& ssid,bool isGroup);
    void sendMessageContentHandler(const QList<MessageContentData>& data);
    void createGroupRequestHandler(const QList<int>& data);

private slots:
    // 图片处理完成后的UI更新槽函数
    void onImageProcessed(const MessageContentData& content);

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

    // 分块接收缓存
    static QMap<QString, QMap<int, QString>> m_fileChunks;
    static QMap<QString, int> m_fileTotalChunks;
    static std::mutex m_chunksMutex;

    QString generateRequestId();
    void cleanupTimeoutRequests();
    void handleFileChunk(const QJsonObject& obj);
    void processCompleteFile(const QString& fileId, const QString& fullData, const QJsonObject& originalObj);
};

#endif // CLIENTREQUESTHANDLER_H
