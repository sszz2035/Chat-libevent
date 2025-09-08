#include "clientconserver.h"
#include"page-view/landpage.h"

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

    connect(socket,SIGNAL(readyRead()),this,SLOT(client_reply_info()));

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
    socket->read(buf,4);
    memcpy(&size,buf,4);
    while(flag)
    {
        memset(buf,0,1024);
        sum+=socket->read(buf,size-sum);
        if(sum>=size)   flag=false;
    }
    ba.append(buf);
    qDebug()<<"data:"<<ba;
}

void ClientConServer::client_reply_info()
{
    QByteArray ba;
    client_receive_data(ba);
    QJsonObject obj;
    obj=QJsonDocument::fromJson(ba).object();
    if(obj.value("cmd").toString()=="register_reply")
    {
        RegisterPage::getInstance()->register_handler(obj);
    }
    else if(obj.value("cmd").toString()=="login_reply")
    {
        LandPage::getInstance()->login_handler(obj);
    }
}

void ClientConServer::handle_error(QAbstractSocket::SocketError error)
{
    qDebug() << "Error occurred: " << error;

    if (error == QAbstractSocket::ConnectionRefusedError ||
        error == QAbstractSocket::NetworkError) {
        if (m_currentRetry < m_maxRetries) {
            qDebug() << "连接失败，将在" << (m_retryInterval / 1000) << "秒后重连";
            m_reconnectTimer.start(m_retryInterval);
            m_currentRetry++;
            ElaMessageBar::error(ElaMessageBarType::Top,"⚠","请检查网络连接！",1000,LandPage::getInstance());
        }
        else {
            qDebug() << "已达到最大尝试次数，放弃连接";
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
    QJsonDocument doc(obj);
    QByteArray sendData=doc.toJson();
    int size=sendData.size();
    sendData.insert(0,(char*)&size,4);
    socket->write(sendData);
}

QTcpSocket *ClientConServer::getClientSocket()
{
    return socket;
}



