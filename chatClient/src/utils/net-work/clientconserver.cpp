#include "clientconserver.h"
ClientConServer * ClientConServer::con = nullptr;

ClientConServer::ClientConServer():socket(new QTcpSocket())
{
    connect(socket,&QTcpSocket::errorOccurred,this,[&](QTcpSocket::SocketError err) {
              qDebug() << "Error occurred: " << err;
    });
}

ClientConServer::~ClientConServer()
{
    socket->deleteLater();
    socket=nullptr;
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



