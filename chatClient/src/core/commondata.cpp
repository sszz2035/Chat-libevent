#include "commondata.h"
#include<mutex>
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


CommonData::CommonData() {}

CommonData::~CommonData()
{

}
