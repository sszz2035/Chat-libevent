#include "loginverify.h"

LoginVerify::LoginVerify() {}

void LoginVerify::setUserName(const QString &name)
{
    userName=name;
}

void LoginVerify::setPassWord(const QString &password)
{
    passWord=password;
}

void LoginVerify::verifyServer()
{
    QJsonObject obj;
    obj.insert("cmd","login");
    obj.insert("username",userName);
    obj.insert("password",passWord);
    ClientConServer::getInstance()->clinet_write_data(obj);
}
