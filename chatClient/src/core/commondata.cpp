#include "commondata.h"
#include<mutex>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

CommonData* CommonData::instance=nullptr;
static std::mutex m;
CommonData *CommonData::getInstance()
{
    if(instance==nullptr)
    {
        m.lock();
        if(instance==nullptr)
        {
            instance=new CommonData();
        }
        m.unlock();
    }
    return instance;
}

void CommonData::destroyInstance()
{
    if(instance)
    {
        m.lock();
        if(instance)
        {
            delete instance;
            instance = nullptr;
        }
        m.unlock();
    }
}

UserBaseInfoData CommonData::getCurUserInfo() const
{
    return _userinfo;
}

void CommonData::setCurUserInfo(const UserBaseInfoData &curUserInfo)
{
    _userinfo=curUserInfo;
}

bool CommonData::setMessageContentData(const QList<MessageContentData> &data, bool isFromRemote)
{
    if(!isFromRemote)
    {
        emit sigMsgContentData(data);
        return true;
    }
    return false;
}

QString CommonData::getMsgPicTempPath() const
{
    return _msgPicTempPath;
}

bool CommonData::saveImageToTemp(const QString& imageName, const QImage& image)
{
    QString filePath = getImageTempFilePath(imageName);
    return image.save(filePath, "PNG");
}

QString CommonData::getImageTempFilePath(const QString& imageName) const
{
    return _msgPicTempPath + "/" + imageName + ".png";
}

CommonData::CommonData() {
    // 初始化图片临时目录
    _msgPicTempPath = QCoreApplication::applicationDirPath() + "/temp/msgpic";
    QDir dir(_msgPicTempPath);
    if (!dir.exists()) {
        dir.mkpath(_msgPicTempPath);
    }
}

CommonData::~CommonData()
{

}
