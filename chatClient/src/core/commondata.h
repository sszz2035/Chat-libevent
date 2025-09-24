#ifndef COMMONDATA_H
#define COMMONDATA_H

#include <QObject>
#include"pagedata.h"
class CommonData:public QObject
{
    Q_OBJECT
public:
    static CommonData * getInstance();
    void destroyInstance();
    UserBaseInfoData getCurUserInfo()const;
    void             setCurUserInfo(const UserBaseInfoData& curUserInfo);
    bool             setMessageContentData(const QList<MessageContentData>& data,bool isFromRemote=false);
private:
    explicit CommonData();
    ~CommonData();
signals:
    void sigMsgContentData(const QList<MessageContentData>& data);
private:
    UserBaseInfoData _userinfo;
    static CommonData * instance;
};

#endif // COMMONDATA_H
