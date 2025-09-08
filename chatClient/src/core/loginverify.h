#ifndef LOGINVERIFY_H
#define LOGINVERIFY_H
#include <QJsonObject>
#include <QObject>
#include "page-view/define.h"
#include"utils/net-work/clientconserver.h"
class LoginVerify
{
public:
    explicit LoginVerify();
    void setSsid(const int& ssid);
    void setPassWord(const QString& password);
    void verifyServer();

private:
    int ssid;
    QString passWord;
};

#endif // LOGINVERIFY_H
