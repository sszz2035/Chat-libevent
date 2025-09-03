#include "clientconserver.h"
ClientConServer::ClientConServer():socket(new QTcpSocket())
{
    connect(socket,&QTcpSocket::errorOccurred,this,[&](QTcpSocket::SocketError err) {
              qDebug() << "Error occurred: " << err;
    });
}

ClientConServer::~ClientConServer()
{
    delete socket;
}

void ClientConServer::connectToServer()
{
    socket->connectToHost(QHostAddress(IP),PORT);
}

QTcpSocket *ClientConServer::getClientSocket()
{
    return socket;
}



