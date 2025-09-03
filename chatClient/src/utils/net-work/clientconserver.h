#ifndef CLIENTCONSERVER_H
#define CLIENTCONSERVER_H

#include <QObject>
#include <QTcpSocket>
#define IP "192.168.200.130"
#define PORT 8888
class ClientConServer:QObject
{
    Q_OBJECT
public:
    ClientConServer();
    ~ClientConServer();
    void connectToServer();

    QTcpSocket* getClientSocket();

private slots:


private:
    QTcpSocket* socket;
};

#endif // CLIENTCONSERVER_H
