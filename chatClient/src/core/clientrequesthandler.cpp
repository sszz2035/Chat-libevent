#include "clientrequesthandler.h"
#include<mutex>
#include<QRandomGenerator>
#include<QDateTime>
#include"utils/log/logfile.h"
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

void ClientRequestHandler::client_reply_info()
{
    QByteArray ba;
    ClientConServer::getInstance()->client_receive_data(ba);
    if (ba.isEmpty()) {
        return; // 没有收到有效数据
    }
    QJsonObject obj;
    obj=QJsonDocument::fromJson(ba).object();
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

    else if(obj.value("cmd").toString()=="query_uid_reply")
    {
        QString requestId = obj.value("request_id").toString();
        QueryCallback callback;
        
        {
            std::lock_guard<std::mutex> lock(m_requestsMutex);
            if (m_pendingRequests.contains(requestId)) {
                callback = m_pendingRequests.take(requestId);
            }
        }
        
        if (callback) {
            callback(obj);
        }
        else {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Received response for unknown request: " + requestId);
        }
        
        // 如果没有待处理请求，停止定时器
        {
            std::lock_guard<std::mutex> lock(m_requestsMutex);
            if (m_pendingRequests.isEmpty()) {
                m_timeoutTimer.stop();
            }
        }
    }
}

ClientRequestHandler::ClientRequestHandler()
{
    // 设置超时定时器
    m_timeoutTimer.setSingleShot(false);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &ClientRequestHandler::cleanupTimeoutRequests);
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
