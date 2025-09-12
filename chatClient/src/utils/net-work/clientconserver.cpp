#include "clientconserver.h"
#include"page-view/landpage.h"
#include"utils/log/logfile.h"

#include <ElaMessageBar.h>

ClientConServer * ClientConServer::con = nullptr;

ClientConServer::ClientConServer():socket(new QTcpSocket()),
    m_maxRetries(5),
    m_currentRetry(0),
    m_retryInterval(10000)
{
    connect(&m_reconnectTimer,&QTimer::timeout,this,[this]{
        this->connectToServer();
    });

    // connect(socket,&QAbstractSocket::errorOccurred,this,&ClientConServer::handle_error);

    connect(socket,SIGNAL(errorOccurred(QAbstractSocket::SocketError)),this,SLOT(handle_error(QAbstractSocket::SocketError)));

    connect(socket,SIGNAL(readyRead()),this,SLOT(readyReadHandler()));

    connect(socket,&QTcpSocket::connected,this,[this]{
        if(socket->state()==QAbstractSocket::ConnectedState)
        {
            LandPage::getInstance()->isFreezeSignInBtn(false);
            m_currentRetry = 0; // 重置重试计数器
            m_reconnectTimer.stop();
        }
    });
}

ClientConServer::~ClientConServer()
{
    socket->deleteLater();
    socket=nullptr;
}

void ClientConServer::client_receive_data(QByteArray &ba)
{
    int size=0,sum=0;
    bool flag=true;
    char buf[1024]={0};
    
    // 检查是否有足够的数据可读
    if (socket->bytesAvailable() < 4) {
        SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Not enough data available for size header");
        return;
    }
    
    qint64 bytesRead = socket->read(buf,4);
    if (bytesRead != 4) {
        SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to read size header, read " + QString::number(bytesRead) + " bytes");
        return;
    }
    
    memcpy(&size,buf,4);
    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Expecting data size: " + QString::number(size) + " bytes, available: " + QString::number(socket->bytesAvailable()) + " bytes");
    
    if (size > 1024 || size <= 0) {
        SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Invalid data size: " + QString::number(size));
        return;
    }
    
    while(flag)
    {
        memset(buf,0,1024);
        int toRead = qMin(size - sum, 1024);
        bytesRead = socket->read(buf, toRead);
        if (bytesRead <= 0) {
            SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to read data, read " + QString::number(bytesRead) + " bytes");
            break;
        }
        sum += bytesRead;
        ba.append(buf, bytesRead);
        if(sum >= size)   flag=false;
    }
    
    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Received complete data: " + QString::number(ba.size()) + " bytes, data: " + ba);
}

void ClientConServer::readyReadHandler()
{
    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "readyRead triggered, bytes available: " + QString::number(socket->bytesAvailable()));
    
    // 循环处理所有可用的数据包
    while (socket->bytesAvailable() >= 4) {
        ClientRequestHandler::getInstance()->client_reply_info();
    }
}

void ClientConServer::handle_error(QAbstractSocket::SocketError error)
{
    SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Error occurred: " + QString::number(error));

    if (error == QAbstractSocket::ConnectionRefusedError ||
        error == QAbstractSocket::NetworkError) {
        if (m_currentRetry < m_maxRetries) {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "连接失败，将在" + QString::number(m_retryInterval / 1000) + "秒后重连");
            m_reconnectTimer.start(m_retryInterval);
            m_currentRetry++;
            ElaMessageBar::error(ElaMessageBarType::Top,"⚠","请检查网络连接！",1000,LandPage::getInstance());
        }
        else {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "已达到最大尝试次数，放弃连接");
            m_reconnectTimer.stop();
            ElaMessageBar::error(ElaMessageBarType::Top,"⚠","放弃连接！",1000,LandPage::getInstance());
            }
    }
    else
    {
        LandPage::getInstance()->isFreezeSignInBtn(true);
        m_reconnectTimer.stop();
        ElaMessageBar::error(ElaMessageBarType::Top,"⚠","服务器繁忙，请稍后重试！",1000,LandPage::getInstance());
    }
}

ClientConServer* ClientConServer::getInstance()
{
    if(!con)
    {
        con=new ClientConServer();
    }
    return con;
}

void ClientConServer::destroyInstance()
{
    if(con)
    {
        delete con;
        con=nullptr;
    }
}

void ClientConServer::connectToServer()
{
    if(socket->state()!=QAbstractSocket::UnconnectedState)  abort();
    socket->connectToHost(QHostAddress(IP),PORT);
}

void ClientConServer::clinet_write_data(QJsonObject &obj)
{
    // 检查连接状态
    if (socket->state() != QAbstractSocket::ConnectedState) {
        SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Socket not connected, current state: " + QString::number(socket->state()));
        return;
    }
    
    QJsonDocument doc(obj);
    QByteArray sendData=doc.toJson();
    int size=sendData.size();
    sendData.insert(0,(char*)&size,4);
    
    // 确保数据完全发送
    qint64 bytesWritten = socket->write(sendData);
    if (bytesWritten == -1) {
        SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to write data: " + socket->errorString());
        return;
    }
    
    // 等待数据发送完成
    socket->flush();
    
    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Sent data: " + obj["cmd"].toString() + " size: " + QString::number(bytesWritten) + " bytes");
}

QTcpSocket *ClientConServer::getClientSocket()
{
    return socket;
}



