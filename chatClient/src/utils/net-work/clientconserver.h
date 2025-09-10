#ifndef CLIENTCONSERVER_H
#define CLIENTCONSERVER_H

#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
#include<QTimer>
// #include<core/dataquery.h>
#include"page-view/registerpage.h"
#include "core/clientrequesthandler.h"

#define IP "192.168.200.130"
#define PORT 8888
class ClientConServer:QObject
{
    Q_OBJECT
public:
    static ClientConServer* getInstance();
    static void destroyInstance();
    void connectToServer();
    void clinet_write_data(QJsonObject& obj);
    void client_receive_data(QByteArray& ba);
    QTcpSocket* getClientSocket();
private:
    explicit ClientConServer();
    ~ClientConServer();

private slots:
    void handle_error(QAbstractSocket::SocketError error);
    void readyReadHandler();
private:
    QTcpSocket* socket;
    static ClientConServer* con;
    int m_maxRetries;      // 最大重试次数
    int m_currentRetry;    // 当前重试计数
    int m_retryInterval;   // 重试间隔（毫秒）
    QTimer m_reconnectTimer; // 重连定时器
};
#endif // CLIENTCONSERVER_H
