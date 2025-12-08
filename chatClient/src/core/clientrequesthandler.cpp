#include "clientrequesthandler.h"
#include<mutex>
#include<QRandomGenerator>
#include<QDateTime>
#include <QJsonArray>
#include <QFile>  // 添加QFile头文件
#include <QThread>  // 添加QThread支持
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>  // 添加QtConcurrent支持异步处理
#include "core/commondata.h"
#include "page-view/contactpage.h"
#include"page-view/messagepage.h"
#include"utils/log/logfile.h"
#include"page-view/archpage.h"
static std::mutex m;

// 分块缓存 - 在类外部定义以避免模板问题
QMap<QString, QMap<int, QString>> ClientRequestHandler::m_fileChunks;
QMap<QString, int> ClientRequestHandler::m_fileTotalChunks;
std::mutex ClientRequestHandler::m_chunksMutex;

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

void ClientRequestHandler::client_reply_info(const QByteArray& ba)
{
    if (ba.isEmpty()) {
        return; // 没有收到有效数据
    }

    // 快速检测是否是图片消息（不解析整个JSON）
    // 图片消息通常很大，通过检查cmd字段快速判断
    bool isImageMessage = ba.contains("\"cmd\":\"image_private\"") ||
                          ba.contains("\"cmd\":\"image_group\"") ||
                          ba.contains("\"cmd\": \"image_private\"") ||
                          ba.contains("\"cmd\": \"image_group\"") ||
                          ba.contains("\"cmd\":\"image_private_chunk\"") ||
                          ba.contains("\"cmd\":\"image_group_chunk\"") ||
                          ba.contains("\"cmd\": \"image_private_chunk\"") ||
                          ba.contains("\"cmd\": \"image_group_chunk\"");

    if (isImageMessage) {
        // 所有图片消息都放到后台线程处理（包括JSON解析）
        // 快速提取request_id用于日志（不完整解析）
        int reqIdStart = ba.indexOf("\"request_id\"");
        QString reqIdLog = reqIdStart > 0 ? "found" : "none";
        SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
            "Received image message, request_id: " + reqIdLog + ", size: " + QString::number(ba.size()) + " bytes, processing in background");

        ClientRequestHandler* handler = this;
        QtConcurrent::run([handler, ba]() {
            // 在后台线程解析JSON
            QJsonObject obj = QJsonDocument::fromJson(ba).object();
            QString cmd = obj.value("cmd").toString();

            // 检查是否是分块命令
            if (cmd.endsWith("_chunk")) {
                handler->handleFileChunk(obj);
                return;
            }

            if (cmd == "image_private") {
                QString fileId = obj["file_id"].toString();
                QString fileData = obj["file_data"].toString();
                int fromFriend = obj["fromfriend"].toInt();
                int curUserSSID = CommonData::getInstance()->getCurUserInfo().ssid;

                if(fileId.isEmpty() || fileData.isEmpty()) {
                    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Invalid image data received");
                    return;
                }

                // 解码Base64数据并创建图片
                QByteArray imageData = QByteArray::fromBase64(fileData.toLocal8Bit());
                if(imageData.isEmpty()) {
                    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to decode Base64 image data");
                    return;
                }

                QImage image = QImage::fromData(imageData);
                if(image.isNull()) {
                    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to create image from data");
                    return;
                }

                // 限制图片分辨率
                if(image.width() > 4096 || image.height() > 4096) {
                    image = image.scaled(4096, 4096, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }

                // 保存到临时目录
                if(!CommonData::getInstance()->saveImageToTemp(fileId, image)) {
                    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to save image to temp");
                    return;
                }

                // 创建消息内容
                MessageContentData content;
                content.contentType = ContentType::Image;
                content.senderSSID = fromFriend;
                content.createTime = 1;
                content.recipient.recipientType = 1;
                content.recipient.recipientSSID = curUserSSID;
                content.fileId = {fileId};
                QString imageUrl = CommonData::getInstance()->getImageTempFilePath(fileId);
                content.content = QString("<img src=\"%1\" style=\"max-width: 300px; max-height: 300px;\">").arg(imageUrl);

                emit handler->imageProcessed(content);
            }
            else if (cmd == "image_group") {
                QString fileId = obj["file_id"].toString();
                QString fileData = obj["file_data"].toString();
                int fromUid = obj["from_uid"].toInt();
                int gid = obj["gid"].toInt();

                if(fileId.isEmpty() || fileData.isEmpty()) {
                    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Invalid group image data received");
                    return;
                }

                // 解码Base64数据并创建图片
                QByteArray imageData = QByteArray::fromBase64(fileData.toLocal8Bit());
                if(imageData.isEmpty()) {
                    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to decode Base64 group image data");
                    return;
                }

                QImage image = QImage::fromData(imageData);
                if(image.isNull()) {
                    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to create group image from data");
                    return;
                }

                // 限制图片分辨率
                if(image.width() > 4096 || image.height() > 4096) {
                    image = image.scaled(4096, 4096, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }

                // 保存到临时目录
                if(!CommonData::getInstance()->saveImageToTemp(fileId, image)) {
                    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to save group image to temp");
                    return;
                }

                // 创建消息内容
                MessageContentData content;
                content.contentType = ContentType::Image;
                content.senderSSID = fromUid;
                content.createTime = 1;
                content.recipient.recipientType = 2;
                content.recipient.recipientSSID = gid;
                content.fileId = {fileId};
                QString imageUrl = CommonData::getInstance()->getImageTempFilePath(fileId);
                content.content = QString("<img src=\"%1\" style=\"max-width: 300px; max-height: 300px;\">").arg(imageUrl);

                emit handler->imageProcessed(content);
            }
        });
        return;  // 图片消息异步处理，直接返回
    }

    // 非图片消息，正常解析
    QJsonObject obj = QJsonDocument::fromJson(ba).object();

    // 打印接收到的cmd和request_id
    QString cmd = obj.contains("cmd") ? obj["cmd"].toString() : "unknown";
    QString reqId = obj.contains("request_id") ? obj["request_id"].toString() : "none";
    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
        "Received cmd: " + cmd + ", request_id: " + reqId);

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

    //群聊回复
    else if(obj.value("cmd").toString()=="groupchat_reply")
    {
        MessageContentData data;
        data.contentType=ContentType::Text;
        data.content=obj["text"].toString();
        data.senderSSID=obj["from_uid"].toInt();
        data.createTime=1;
        data.recipient.recipientType=2;
        data.recipient.recipientSSID=obj["gid"].toInt();
        MessagePage::getInstance()->loadCacheMsg({data});
    }

    // 私聊文件传输
    else if(obj.value("cmd").toString()=="transfer_file")
    {
        QString fileId = obj["file_id"].toString();
        QString fileData = obj["file_data"].toString();
        QString fileName = obj["file_name"].toString();
        int fileSize = obj["file_size"].toInt();

        if(fileId.isEmpty() || fileData.isEmpty()) {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Invalid file data received");
            return;
        }

        // 检查是私聊还是群聊
        if (obj.contains("fromfriend")) {
            // 私聊文件
            int fromFriend = obj["fromfriend"].toInt();
            int curUserSSID = CommonData::getInstance()->getCurUserInfo().ssid;

            // 解码Base64数据
            QByteArray fileByteData = QByteArray::fromBase64(fileData.toLocal8Bit());
            if(fileByteData.isEmpty()) {
                SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to decode Base64 file data");
                return;
            }

            // 保存文件到项目file_storage目录
            QString storageDir = QCoreApplication::applicationDirPath() + "/file_storage";
            QDir dir;
            if (!dir.exists(storageDir)) {
                dir.mkpath(storageDir);
            }
            QString filePath = storageDir + "/" + fileId;
            QFile outFile(filePath);
            if (outFile.open(QIODevice::WriteOnly)) {
                outFile.write(fileByteData);
                outFile.close();
                SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
                    QString("Saved file %1 (%2 bytes) to file_storage").arg(fileName).arg(fileSize));
            } else {
                SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to save file to file_storage");
                return;
            }

            // 创建消息内容
            MessageContentData content;
            content.contentType = ContentType::File;
            content.senderSSID = fromFriend;
            content.createTime = 1;
            content.recipient.recipientType = 1;
            content.recipient.recipientSSID = curUserSSID;
            content.fileId = {fileId};
            content.content = fileName;

            MessagePage::getInstance()->loadCacheMsg({content});
        }
        else if (obj.contains("from_uid")) {
            // 群聊文件
            int fromUid = obj["from_uid"].toInt();
            int gid = obj["gid"].toInt();

            // 解码Base64数据
            QByteArray fileByteData = QByteArray::fromBase64(fileData.toLocal8Bit());
            if(fileByteData.isEmpty()) {
                SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to decode Base64 group file data");
                return;
            }

            // 保存文件到项目file_storage目录
            QString storageDir = QCoreApplication::applicationDirPath() + "/file_storage";
            QDir dir;
            if (!dir.exists(storageDir)) {
                dir.mkpath(storageDir);
            }
            QString filePath = storageDir + "/" + fileId;
            QFile outFile(filePath);
            if (outFile.open(QIODevice::WriteOnly)) {
                outFile.write(fileByteData);
                outFile.close();
                SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
                    QString("Saved group file %1 (%2 bytes) to file_storage").arg(fileName).arg(fileSize));
            } else {
                SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to save group file to file_storage");
                return;
            }

            // 创建消息内容
            MessageContentData content;
            content.contentType = ContentType::File;
            content.senderSSID = fromUid;
            content.createTime = 1;
            content.recipient.recipientType = 2;
            content.recipient.recipientSSID = gid;
            content.fileId = {fileId};
            content.content = fileName;

            MessagePage::getInstance()->loadCacheMsg({content});
        }
    }

    // 私聊图片回复
    else if(obj.value("cmd").toString()=="image_private_reply")
    {
        QString result = obj["result"].toString();
        if(result == "success")
        {
            SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Image sent successfully");
        }
        else if(result == "offline")
        {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Friend is offline, image not sent");
        }
        else if(result == "save_failed")
        {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to save image on server");
        }
    }

    // 群聊图片回复
    else if(obj.value("cmd").toString()=="image_group_reply")
    {
        QString result = obj["result"].toString();
        if(result == "success")
        {
            SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Group image sent successfully");
        }
        else if(result == "save_failed")
        {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to save group image on server");
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

        // 判断消息类型
        if(content.contentType == ContentType::Image)
        {
            // 处理图片消息
            if(content.recipient.recipientType==1)
            {
                // 私聊图片
                for(const QString& fileId : content.fileId)
                {
                    QString imagePath = CommonData::getInstance()->getImageTempFilePath(fileId);
                    QFile file(imagePath);
                    if(file.open(QIODevice::ReadOnly))
                    {
                        QByteArray imageData = file.readAll();
                        file.close();

                        // 转换为Base64
                        QByteArray base64Data = imageData.toBase64();

                        QJsonObject send_data;
                        send_data.insert("cmd","image_private");
                        send_data.insert("request_id",requestId);
                        send_data.insert("uid",content.senderSSID);
                        send_data.insert("tofriend",content.recipient.recipientSSID);
                        send_data.insert("file_id",fileId);
                        send_data.insert("file_data",QString(base64Data));
                        ClientConServer::getInstance()->clinet_write_data(send_data);
                    }
                }
            }
            else if(content.recipient.recipientType==2)
            {
                // 群聊图片
                for(const QString& fileId : content.fileId)
                {
                    QString imagePath = CommonData::getInstance()->getImageTempFilePath(fileId);
                    QFile file(imagePath);
                    if(file.open(QIODevice::ReadOnly))
                    {
                        QByteArray imageData = file.readAll();
                        file.close();

                        // 转换为Base64
                        QByteArray base64Data = imageData.toBase64();

                        QJsonObject send_data;
                        send_data.insert("cmd","image_group");
                        send_data.insert("request_id",requestId);
                        send_data.insert("uid",content.senderSSID);
                        send_data.insert("gid",content.recipient.recipientSSID);
                        send_data.insert("file_id",fileId);
                        send_data.insert("file_data",QString(base64Data));
                        ClientConServer::getInstance()->clinet_write_data(send_data);
                    }
                }
            }
        }
        else if(content.contentType == ContentType::File)
        {
            // 处理文件传输
            if(content.recipient.recipientType==1)
            {
                // 私聊文件
                for(const QString& fileId : content.fileId)
                {
                    QString storageDir = QCoreApplication::applicationDirPath() + "/file_storage";
                    QString filePath = storageDir + "/" + fileId;
                    QFile file(filePath);
                    if(file.open(QIODevice::ReadOnly))
                    {
                        QByteArray fileData = file.readAll();
                        file.close();

                        // 转换为Base64
                        QByteArray base64Data = fileData.toBase64();

                        QJsonObject send_data;
                        send_data.insert("cmd","transfer_file");
                        send_data.insert("request_id",requestId);
                        send_data.insert("uid",content.senderSSID);
                        send_data.insert("tofriend",content.recipient.recipientSSID);
                        send_data.insert("file_id",fileId);
                        send_data.insert("file_data",QString(base64Data));
                        send_data.insert("file_name",content.content);
                        send_data.insert("file_size",(int)fileData.size());
                        ClientConServer::getInstance()->clinet_write_data(send_data);
                    }
                }
            }
            else if(content.recipient.recipientType==2)
            {
                // 群聊文件
                for(const QString& fileId : content.fileId)
                {
                    QString storageDir = QCoreApplication::applicationDirPath() + "/file_storage";
                    QString filePath = storageDir + "/" + fileId;
                    QFile file(filePath);
                    if(file.open(QIODevice::ReadOnly))
                    {
                        QByteArray fileData = file.readAll();
                        file.close();

                        // 转换为Base64
                        QByteArray base64Data = fileData.toBase64();

                        QJsonObject send_data;
                        send_data.insert("cmd","transfer_file");
                        send_data.insert("request_id",requestId);
                        send_data.insert("uid",content.senderSSID);
                        send_data.insert("gid",content.recipient.recipientSSID);
                        send_data.insert("file_id",fileId);
                        send_data.insert("file_data",QString(base64Data));
                        send_data.insert("file_name",content.content);
                        send_data.insert("file_size",(int)fileData.size());
                        ClientConServer::getInstance()->clinet_write_data(send_data);
                    }
                }
            }
        }
        else
        {
            // 处理文本消息
            if(content.recipient.recipientType==1)
            {
                QJsonObject send_data;
                //好友私聊
                send_data.insert("cmd","private");
                send_data.insert("request_id",requestId);
                send_data.insert("uid",content.senderSSID);
                send_data.insert("tofriend",content.recipient.recipientSSID);
                send_data.insert("text",content.content);
                ClientConServer::getInstance()->clinet_write_data(send_data);
            }

            else if(content.recipient.recipientType==2)
            {
                QJsonObject send_data;
                //群聊
                send_data.insert("cmd","groupchat");
                send_data.insert("request_id",requestId);
                send_data.insert("uid",content.senderSSID);
                send_data.insert("gid",content.recipient.recipientSSID);
                send_data.insert("text",content.content);
                ClientConServer::getInstance()->clinet_write_data(send_data);
            }
        }
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

    // 连接图片处理完成信号到UI更新槽（Qt::QueuedConnection确保在主线程执行）
    connect(this, &ClientRequestHandler::imageProcessed, this, &ClientRequestHandler::onImageProcessed, Qt::QueuedConnection);
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

// 图片处理完成后在主线程更新UI
void ClientRequestHandler::onImageProcessed(const MessageContentData& content)
{
    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Updating UI with processed image");
    MessagePage::getInstance()->loadCacheMsg({content});
}

// 处理分块接收
void ClientRequestHandler::handleFileChunk(const QJsonObject& obj)
{
    QString fileId = obj["file_id"].toString();
    int chunkIndex = obj["chunk_index"].toInt();
    int totalChunks = obj["total_chunks"].toInt();
    QString chunkData = obj["chunk_data"].toString();
    QString cmd = obj["cmd"].toString();

    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
        QString("Received chunk %1/%2 for file %3").arg(chunkIndex + 1).arg(totalChunks).arg(fileId));

    // 缓存分块数据
    {
        std::lock_guard<std::mutex> lock(m_chunksMutex);
        m_fileChunks[fileId][chunkIndex] = chunkData;
        m_fileTotalChunks[fileId] = totalChunks;
    }

    // 检查是否所有分块都接收完毕
    {
        std::lock_guard<std::mutex> lock(m_chunksMutex);
        if (m_fileChunks[fileId].size() == totalChunks) {
            SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
                QString("All %1 chunks received for file %2, assembling...").arg(totalChunks).arg(fileId));

            // 组装完整数据
            QString fullData;
            for (int i = 0; i < totalChunks; i++) {
                fullData += m_fileChunks[fileId][i];
            }

            // 清理缓存
            m_fileChunks.remove(fileId);
            m_fileTotalChunks.remove(fileId);

            // 在后台线程处理完整文件
            QtConcurrent::run([this, fileId, fullData, cmd, obj]() {
                processCompleteFile(fileId, fullData, obj);
            });
        }
    }
}

// 处理完整文件（组装完成后调用）
void ClientRequestHandler::processCompleteFile(const QString& fileId, const QString& fullData, const QJsonObject& originalObj)
{
    QString cmd = originalObj["cmd"].toString();
    cmd.remove("_chunk");  // 移除_chunk后缀

    QJsonObject fullObj = originalObj;
    fullObj["cmd"] = cmd;
    fullObj["file_data"] = fullData;

    // 根据消息类型创建MessageContentData
    MessageContentData content;
    int curUserSSID = CommonData::getInstance()->getCurUserInfo().ssid;

    if (cmd == "image_private") {
        int fromFriend = originalObj["fromfriend"].toInt();

        // 解码Base64数据并创建图片
        QByteArray imageData = QByteArray::fromBase64(fullData.toLocal8Bit());
        if(imageData.isEmpty()) {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to decode Base64 image data from chunks");
            return;
        }

        QImage image = QImage::fromData(imageData);
        if(image.isNull()) {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to create image from assembled data");
            return;
        }

        // 限制图片分辨率
        if(image.width() > 4096 || image.height() > 4096) {
            image = image.scaled(4096, 4096, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // 保存到临时目录
        if(!CommonData::getInstance()->saveImageToTemp(fileId, image)) {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to save assembled image to temp");
            return;
        }

        // 创建消息内容
        content.contentType = ContentType::Image;
        content.senderSSID = fromFriend;
        content.createTime = 1;
        content.recipient.recipientType = 1;
        content.recipient.recipientSSID = curUserSSID;
        content.fileId = {fileId};
        QString imageUrl = CommonData::getInstance()->getImageTempFilePath(fileId);
        content.content = QString("<img src=\"%1\" style=\"max-width: 300px; max-height: 300px;\">").arg(imageUrl);

        SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
            QString("Successfully assembled and processed image %1 (%2 bytes)").arg(fileId).arg(imageData.size()));

        emit imageProcessed(content);
    }
    else if (cmd == "image_group") {
        int fromUid = originalObj["from_uid"].toInt();
        int gid = originalObj["gid"].toInt();

        // 解码Base64数据并创建图片
        QByteArray imageData = QByteArray::fromBase64(fullData.toLocal8Bit());
        if(imageData.isEmpty()) {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to decode Base64 group image data from chunks");
            return;
        }

        QImage image = QImage::fromData(imageData);
        if(image.isNull()) {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to create group image from assembled data");
            return;
        }

        // 限制图片分辨率
        if(image.width() > 4096 || image.height() > 4096) {
            image = image.scaled(4096, 4096, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // 保存到临时目录
        if(!CommonData::getInstance()->saveImageToTemp(fileId, image)) {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to save assembled group image to temp");
            return;
        }

        // 创建消息内容
        content.contentType = ContentType::Image;
        content.senderSSID = fromUid;
        content.createTime = 1;
        content.recipient.recipientType = 2;
        content.recipient.recipientSSID = gid;
        content.fileId = {fileId};
        QString imageUrl = CommonData::getInstance()->getImageTempFilePath(fileId);
        content.content = QString("<img src=\"%1\" style=\"max-width: 300px; max-height: 300px;\">").arg(imageUrl);

        SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
            QString("Successfully assembled and processed group image %1 (%2 bytes)").arg(fileId).arg(imageData.size()));

        emit imageProcessed(content);
    }
}
