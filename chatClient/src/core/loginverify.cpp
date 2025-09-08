#include "loginverify.h"

LoginVerify::LoginVerify() {}

void LoginVerify::setSsid(const int &ssid)
{
    this->ssid=ssid;
}

void LoginVerify::setPassWord(const QString &password)
{
    passWord=password;
}

void LoginVerify::verifyServer()
{
    QJsonObject obj;
    obj.insert("cmd","login");
    obj.insert("uid",ssid);
    obj.insert("password",passWord);
    ClientConServer::getInstance()->clinet_write_data(obj);
}
