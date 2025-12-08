#ifndef COMMONDATA_H
#define COMMONDATA_H

#include <QObject>
#include <QImage>
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

    // 图片临时目录管理
    QString getMsgPicTempPath() const;
    bool saveImageToTemp(const QString& imageName, const QImage& image);
    QString getImageTempFilePath(const QString& imageName) const;

private:
    explicit CommonData();
    ~CommonData();
signals:
    //在clientrequesthandler中被处理
    void sigMsgContentData(const QList<MessageContentData>& data);
    void sigCreateGroupRequest(const QList<int>& data);

private:
    UserBaseInfoData _userinfo;
    static CommonData * instance;
    QString _msgPicTempPath;
};

#endif // COMMONDATA_H
