#ifndef COMMONDATA_H
#define COMMONDATA_H

#include <QObject>
#include"pagedata.h"
class CommonData:QObject
{
    Q_OBJECT
public:
    static CommonData * getInstance();
    void destroyInstance();
    UserBaseInfoData getCurUserInfo()const;
    void             setCurUserInfo(const UserBaseInfoData& curUserInfo);
private:
    explicit CommonData();
    ~CommonData();
private:
    UserBaseInfoData _userinfo;
    static CommonData * instance;
};

#endif // COMMONDATA_H
