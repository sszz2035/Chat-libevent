#include "clientrequesthandler.h"
#include<mutex>
#include<QRandomGenerator>
#include<QDateTime>
#include <QJsonArray>
#include "core/commondata.h"
#include "page-view/contactpage.h"
#include"page-view/messagepage.h"
#include"utils/log/logfile.h"
#include"page-view/archpage.h"
static std::mutex m;

ClientRequestHandler* ClientRequestHandler::instance=nullptr;
ClientRequestHandler *ClientRequestHandler::getInstance()
{
    if(instance == nullptr)
    {
        m.lock();
        if(instance == nullptr)
        {
            instance = new ClientRequestHandler();
        }
        m.unlock();
    }
    return instance;
}

void ClientRequestHandler::destroyInstance()
{
    if(instance)
    {
        m.lock();
        if(instance)
        {
            delete instance;
            instance = nullptr;
        }
        m.unlock();
    }
}

void ClientRequestHandler::queryUserInfoByUid(const qint32 &uid, QueryCallback callback)
{
    QString requestId = generateRequestId();
    
    // 存储回调函数
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        m_pendingRequests[requestId] = callback;
    }
    
    // 发送请求数据
    QJsonObject send_data;
    send_data["cmd"] = "query_by_uid";
    send_data["uid"] = uid;
    send_data["request_id"] = requestId;
    
    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Sending request: " + requestId + " for uid: " + QString::number(uid));
    ClientConServer::getInstance()->clinet_write_data(send_data);
    
    // 启动超时检查
    if (!m_timeoutTimer.isActive()) {
        m_timeoutTimer.start(5000); // 5秒超时检查
        SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Started timeout check timer");
    }
}

void ClientRequestHandler::queryGroupInfoByGid(const qint32 &gid, QueryCallback callback)
{
    QString requestId = generateRequestId();

    // 存储回调函数
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        m_pendingRequests[requestId] = callback;
    }

    // 发送请求数据
    QJsonObject send_data;
    send_data["cmd"] = "query_by_gid";
    send_data["gid"] = gid;
    send_data["request_id"] = requestId;

    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Sending request: " + requestId + " for gid: " + QString::number(gid));
    ClientConServer::getInstance()->clinet_write_data(send_data);

    // 启动超时检查
    if (!m_timeoutTimer.isActive()) {
        m_timeoutTimer.start(5000); // 5秒超时检查
        SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Started timeout check timer");
    }
}

void ClientRequestHandler::queryUserInfoBatch(const std::vector<qint32> &uids, QueryCallback callback)
{
    QString requestId = generateRequestId();
    // 存储回调函数
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        m_pendingRequests[requestId] = callback;
    }

    QJsonArray queryArray;
    for(const qint32 val:uids)
    {
        queryArray.append(val);
    }
    QJsonObject sendData;
    sendData["cmd"]="query_users_batch";
    sendData["uids"]=queryArray;
    sendData["request_id"]=requestId;
    ClientConServer::getInstance()->clinet_write_data(sendData);

    // 启动超时检查
    if (!m_timeoutTimer.isActive()) {
        m_timeoutTimer.start(5000); // 5秒超时检查
        SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Started timeout check timer");
    }
}

void ClientRequestHandler::client_reply_info()
{
    QByteArray ba;
    ClientConServer::getInstance()->client_receive_data(ba);
    if (ba.isEmpty()) {
        return; // 没有收到有效数据
    }
    QJsonObject obj;
    obj=QJsonDocument::fromJson(ba).object();

    //取出用户请求
    QueryCallback callback;
    QString requestId =obj.contains("request_id")?obj["request_id"].toString():"null";
    {
        std::lock_guard<std::mutex>lock(m_requestsMutex);
        if(m_pendingRequests.contains(requestId))
        {
            callback=m_pendingRequests.take(requestId);
        }
    }

    //注册回复
    if(obj.value("cmd").toString()=="register_reply")
    {
        RegisterPage::getInstance()->register_handler(obj);
    }
    //登录回复
    else if(obj.value("cmd").toString()=="login_reply")
    {
        LandPage::getInstance()->login_handler(obj);
    }

    //根据id查询用户信息
    else if(obj.value("cmd").toString()=="query_uid_reply")
    {
        QString requestId = obj.value("request_id").toString();

        if (callback) {
            callback(obj);
        }
        else {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Received response for unknown request: " + requestId);
        }
        
    }
    //模糊查询信息回复
    else if(obj.value("cmd").toString()=="search_reply")
    {
        //执行回调函数
        if(callback)
        {
            callback(obj);
        }
        else {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Received response for unknown request: " + requestId);
        }

    }
    //添加好友回复
    else if(obj.value("cmd").toString()=="addfriend_reply")
    {
        //发送回复，在添加好友界面里进行处理数据
        emit addFriendResponse(obj);
    }

    //被添加好友
    else if(obj.value("cmd").toString()=="be_addfriend")
    {

        MsgCombineData info;
        info.userBaseInfo.ssid=obj["uid"].toInt();
        info.userBaseInfo.username=obj["username"].toString();
        ContactPage::getInstance()->addContactInfo("我的好友",info);
    }

    //被私聊
    else if(obj.value("cmd").toString()=="private")
    {
        MessageContentData content;
        content.contentType=ContentType::Text;
        content.content=obj["text"].toString();
        content.senderSSID=obj["fromfriend"].toInt();
        content.createTime=1;
        content.recipient.recipientType=1;
        content.recipient.recipientSSID=obj["tofriend"].toInt();
        MessagePage::getInstance()->loadCacheMsg({content});
    }

    //查询群信息回复
    else if(obj.value("cmd").toString()=="query_gid_reply")
    {
        //执行回调函数
        if(callback)
        {
            callback(obj);
        }
        else {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Received response for unknown request: " + requestId);
        }
    }

    //创建群回复
    else if(obj.value("cmd").toString()=="creategroup_reply")
    {
        //插入联系人界面
        QString groupingName="我加入的群聊";
        FriendshipData data;
        // data.groupBaseInfo.groupName=obj["groupname"].toString();
        // data.groupBaseInfo.ssidGroup=obj["gid"].toInt();
        // data.groupBaseInfo.createSSID=obj["createSSID"].toInt();
        QString groupMember=obj["groupmember"].toString();
        data.ssid=CommonData::getInstance()->getCurUserInfo().ssid;
        data.groupingName=groupingName;
        data.friendType=2;
        data.friendName=obj["groupname"].toString();
        data.friendSSID=obj["gid"].toInt();
        data.groupingName=groupingName;
        data.status=groupMember;
        ContactPage::getInstance()->loadCacheContact({data});
        //创建群聊卡片
    }

    //加入群回复
    else if(obj.value("cmd").toString()=="joingroup_reply")
    {
        //发送回复，在添加群组界面里进行处理数据
        emit addGroupResponse(obj);
    }

    //有新成员加入群回复
    else if(obj.value("cmd").toString()=="new_member_join")
    {
        emit newMemberJoinResponse(obj);
    }

    //批量查询回复
    else if(obj.value("cmd").toString()=="query_users_batch_reply")
    {
        //执行回调函数
        if(callback)
        {
            callback(obj);
        }
        else {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Received response for unknown request: " + requestId);
        }
    }

    //如果没有待处理请求 停止计时器
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        if(m_pendingRequests.isEmpty())
        {
            m_timeoutTimer.stop();
        }
    }

}

void ClientRequestHandler::queryFuzzySearchRequsetHandler(const QString &content, bool isGroup, QueryCallback callback)
{
    //生成请求ID
    QString requestId=generateRequestId();
    //储存回调函数
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        m_pendingRequests[requestId]=callback;
    }
    QJsonObject obj;
    obj.insert("cmd","fuzzy_search");
    obj.insert("request_id",requestId);
    obj.insert("content",content);
    if(isGroup) obj.insert("type","group");
    else        obj.insert("type","friend");
    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Sending request: " + requestId );
    ClientConServer::getInstance()->clinet_write_data(obj);
    //启动超时检查
    if(!m_timeoutTimer.isActive())
    {
        m_timeoutTimer.start(5000);
    }
}

void ClientRequestHandler::addFriendRequestHandler(const qint32 &ssid, bool isGroup)
{
    //生成请求ID
    QString requestId=generateRequestId();

    //将请求ID插入任务队列中
    {
        std::lock_guard<std::mutex>lock(m_requestsMutex);
        m_pendingRequests.insert(requestId,{});
    }
    if(!isGroup)
    {
        QJsonObject send_data;
        send_data["cmd"]="addfriend";
        send_data["uid"]=CommonData::getInstance()->getCurUserInfo().ssid;
        send_data["friend_uid"]=ssid;
        send_data["request_id"]=requestId;
        ClientConServer::getInstance()->clinet_write_data(send_data);
    }

    else
    {
        QJsonObject send_data;
        send_data["cmd"]="joingroup";
        send_data["uid"]=CommonData::getInstance()->getCurUserInfo().ssid;
        send_data["gid"]=ssid;
        send_data["request_id"]=requestId;
        ClientConServer::getInstance()->clinet_write_data(send_data);
    }

    //启动超时检查
    if(!m_timeoutTimer.isActive())
    {
        m_timeoutTimer.start(5000);
    }
}

void ClientRequestHandler::sendMessageContentHandler(const QList<MessageContentData>& data)
{
    for(MessageContentData content:data)
    {
        //生成请求ID
        QString requestId=generateRequestId();
        //将请求ID插入任务队列中
        {
            std::lock_guard<std::mutex>lock(m_requestsMutex);
            m_pendingRequests.insert(requestId,{});
        }

        QJsonObject send_data;
        //好友私聊
        send_data.insert("cmd","private");
        send_data.insert("request_id",requestId);
        send_data.insert("uid",content.senderSSID);
        send_data.insert("tofriend",content.recipient.recipientSSID);
        send_data.insert("text",content.content);
        ClientConServer::getInstance()->clinet_write_data(send_data);
    }
    //启动超时检查
    if(!m_timeoutTimer.isActive())
    {
        m_timeoutTimer.start(5000);
    }
}

void ClientRequestHandler::createGroupRequestHandler(const QList<int> &data)
{
    QString requestId=generateRequestId();
    //将请求ID插入任务队列中
    {
        std::lock_guard<std::mutex>lock(m_requestsMutex);
        m_pendingRequests.insert(requestId,{});
    }

    QJsonObject obj;
    //对成员进行序列化
    QString memberlist;
    qint32 ownerSSID=CommonData::getInstance()->getCurUserInfo().ssid;
    QString ownerName=CommonData::getInstance()->getCurUserInfo().username;
    if(data.size()==1)  memberlist=QString::number(data[0]);
    else
    {
        for(auto member:data)
        {
            memberlist.append(QString::number(member));
            memberlist.append("|");
        }
        memberlist.removeAt(memberlist.size()-1);
    }
    obj.insert("cmd","creategroup");
    obj.insert("request_id",requestId);
    obj.insert("group_member",memberlist);
    obj.insert("owner",ownerSSID);
    obj.insert("ownername",ownerName);
    ClientConServer::getInstance()->clinet_write_data(obj);
    if(!m_timeoutTimer.isActive())
    {
        m_timeoutTimer.start(5000);
    }
}

ClientRequestHandler::ClientRequestHandler()
{
    // 设置超时定时器
    m_timeoutTimer.setSingleShot(false);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &ClientRequestHandler::cleanupTimeoutRequests);

    connect(this,&ClientRequestHandler::queryFuzzySearchRequest,this,&ClientRequestHandler::queryFuzzySearchRequsetHandler);

    connect(CommonData::getInstance(),&CommonData::sigMsgContentData,this,&ClientRequestHandler::sendMessageContentHandler);

    connect(CommonData::getInstance(),&CommonData::sigCreateGroupRequest,this,&ClientRequestHandler::createGroupRequestHandler);
}

ClientRequestHandler::~ClientRequestHandler()
{
    m_timeoutTimer.stop();
    m_pendingRequests.clear();
}

QString ClientRequestHandler::generateRequestId()
{
    return QString::number(QDateTime::currentMSecsSinceEpoch()) + "_" + 
           QString::number(QRandomGenerator::global()->bounded(1000, 9999));
}

void ClientRequestHandler::cleanupTimeoutRequests()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QStringList timeoutRequests;
    
    // 查找超时的请求（超过10秒）
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
            QString requestId = it.key();
            qint64 timestamp = requestId.split("_").first().toLongLong();
            
            if ((now - timestamp) > 10000) {  // 10秒 = 10000毫秒
                timeoutRequests.append(requestId);
            }
        }
        
        // 移除超时请求
        for (const QString& requestId : timeoutRequests) {
            m_pendingRequests.remove(requestId);
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Request timeout: " + requestId);
        }
        
        // 如果没有待处理请求，停止定时器
        if (m_pendingRequests.isEmpty()) {
            m_timeoutTimer.stop();
        }
    }
}
