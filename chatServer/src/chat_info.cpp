#include"chat_info.h"

ChatInfo::ChatInfo()
{
    online_user=new std::list<User>;
    group_info=new std::map<std::string,std::list<User>>;
}

ChatInfo::~ChatInfo()
{
    if(online_user) delete online_user;
    if(group_info) delete group_info;
}