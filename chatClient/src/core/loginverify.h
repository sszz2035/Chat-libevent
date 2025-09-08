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
    void setUserName(const QString& name);
    void setPassWord(const QString& password);
    void verifyServer();

private:
    QString userName;
    QString passWord;
};

#endif // LOGINVERIFY_H
