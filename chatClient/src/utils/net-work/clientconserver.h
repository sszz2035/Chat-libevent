#ifndef CLIENTCONSERVER_H
#define CLIENTCONSERVER_H

#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
#include<QTimer>
#include"page-view/registerpage.h"
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
    QTcpSocket* getClientSocket();
private:
    explicit ClientConServer();
    ~ClientConServer();
    void client_receive_data(QByteArray& ba);

private slots:
    void client_reply_info();
    void handle_error(QAbstractSocket::SocketError error);
private:
    QTcpSocket* socket;
    static ClientConServer* con;
    int m_maxRetries;      // 最大重试次数
    int m_currentRetry;    // 当前重试计数
    int m_retryInterval;   // 重试间隔（毫秒）
    QTimer m_reconnectTimer; // 重连定时器
};
#endif // CLIENTCONSERVER_H
