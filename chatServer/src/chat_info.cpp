#include"chat_info.h"
#include<algorithm>

ChatInfo::ChatInfo()
{
    online_user=new std::list<User>;
    group_info=new std::map<std::string,std::list<std::string>>;
}

ChatInfo::~ChatInfo()
{
    if(online_user) delete online_user;
    if(group_info) delete group_info;
}

// bool User::operator==(const User& other)
// {
//     return name==other.name;
// }

void ChatInfo::list_update_group(std::string* g,int size)
{
    if(size<=0) return;
  
    std::string groupname,membername;
    std::list<std::string>l;
    //遍历g数组
    for(int i=0;i<size;i++)
    {
        int idx=g[i].find('|');
        groupname=g[i].substr(0,idx);
        //提取所有的成员,放入链表l中
        int start=0;
        while(true)
        {
            start=idx+1;
            idx=g[i].find('|',start);
            if(idx==-1) break;
            membername=g[i].substr(start,idx-start);
            l.push_back(membername);
        }
        //处理最后一个成员
        membername=g[i].substr(start);
        l.push_back(membername);
        //将信息插入到group_info中
        group_info->insert(std::pair<std::string,std::list<std::string>>(groupname,l));
        //清空链表
        l.clear();
    }
}

void ChatInfo::list_print_group()
{    
    //上锁
    std::unique_lock<std::mutex>lock(map_mutex);
    for(auto it=group_info->begin();it!=group_info->end();it++)
    {
        std::cout<<it->first<<" ";
        for(auto i=it->second.begin();i!=it->second.end();i++)
        {
            std::cout<<*i<<" ";
        }
        std::cout<<std::endl;
    }
}

bool ChatInfo::list_update_list(struct bufferevent* bev,Json::Value& v)
{
    //上锁
    std::unique_lock<std::mutex>lock(list_mutex);
    User user={v["username"].asString(),bev};
    online_user->push_back(user);
    return true;
}

struct bufferevent* ChatInfo::list_friend_online(const std::string& user)
{
    //上锁
    std::unique_lock<std::mutex>lock(list_mutex);
    auto it=std::find_if(online_user->begin(),online_user->end(),[user](const User& u)->bool{
        return user==u.name;
        });
    if(it!=online_user->end())
    {
        return it->bufevent;
    }
    else
    {
        return NULL;
    }
}

bool ChatInfo::list_group_is_exist(const std::string &groupname)
{
    //上锁
    std::unique_lock<std::mutex>lock(map_mutex);
    auto it=group_info->find(groupname);
    //存在
    if(it!=group_info->end())   return true;
    else    return false;
}

void ChatInfo::list_add_new_group(const std::string &groupname,const std::string& owner)
{
    //上锁
    std::unique_lock<std::mutex>lock(map_mutex);
    std::list<std::string>l;
    l.push_back(owner);
    group_info->insert(std::make_pair(groupname,l));
}
