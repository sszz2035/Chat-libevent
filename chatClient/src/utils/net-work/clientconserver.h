#ifndef CLIENTCONSERVER_H
#define CLIENTCONSERVER_H

#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
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

private slots:


private:
    QTcpSocket* socket;
    static ClientConServer* con;
};
#endif // CLIENTCONSERVER_H
