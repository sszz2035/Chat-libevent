#include"data_base.h"
#include"log.h"
DataBase::DataBase()
{

}

DataBase::~DataBase()
{
    
}

bool DataBase::database_connect()
{
    //上写锁
    std::unique_lock<std::shared_mutex>lock(mutex_);

    //初始化数据库句柄
    MYSQL* mysql_handle=mysql_init(NULL);
    if(mysql_handle==NULL)
    {
        LOG_PERROR("mysql_init_error");
        return false;
    }
    //连接mysql
    if(mysql_real_connect(mysql_handle,"localhost","root","5201314Zqzq!","chat_database",0,NULL,0)==NULL)
    {
        LOG_PERROR("mysql_real_connect");      
        mysql_close(mysql_handle);  
        return false;
    }

    //设置编码格式为utf8mb4
    if(mysql_query(mysql_handle,"SET NAMES utf8mb4")!=0)
    {
        LOG_PERROR("mysql_query");        
        mysql_close(mysql_handle);  
        return false;
    }
    //将指针所有权交给智能指针
    mysql.reset(mysql_handle);
    return true;
}

void DataBase::database_disconnect()
{
    //上写锁
    std::unique_lock<std::shared_mutex> lock(mutex_);
    //释放对象
    mysql.reset();
}

bool DataBase::exec_query_and_fetch_row(const char* sql, MYSQL_ROW& row)
{
    //上读锁
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if(mysql_query(mysql.get(),sql))
    {
        LOG_PERROR("mysql_query");        
        return false;
    }
    //存储结果
     MYSQL_RES* res=mysql_store_result(mysql.get());
     if(res==NULL)
    {
        LOG_PERROR("mysql_store_result");      
        mysql_free_result(res);  
        return false;
    }
    MYSQL_ROW r=mysql_fetch_row(res);
    if(r==NULL)
    {
        LOG_PERROR("mysql_fetch_row");
        return false;
    }
    //释放结果
    mysql_free_result(res);
    //返回结果
    row=r;
    return true;
}

bool DataBase::exec_update(const char* sql)
{
    //上写锁
    std::unique_lock<std::shared_mutex>lock(mutex_);
    if(mysql_query(mysql.get(),sql))
    {
        LOG_PERROR("mysql_query");        
        return false;
    }
    return true;
}

int DataBase::database_get_group_info(std::string* g)
{
    //上读锁
    std::shared_lock<std::shared_mutex> lock(mutex_);

    //查询所有群信息
    if(mysql_query(mysql.get(),"SELECT * FROM chat_group;"))
    {
        LOG_PERROR("mysql_query");        
        return -1;
    }
    //存储结果
    MYSQL_RES* res=mysql_store_result(mysql.get());
    if(res==NULL)
    {
        LOG_PERROR("mysql_store_result");    
        mysql_free_result(res);      
        return -1;
    }
    
    //读取每一行的结果
    MYSQL_ROW r;
    int idx=0;//数组序列号
    while(r=mysql_fetch_row(res))
    {
        g[idx]+=r[0];
        g[idx]+='|';
        g[idx]+=r[2];
        idx++;
    }

    //释放结果
    mysql_free_result(res);
    return idx;
}

bool DataBase::database_init_table()
{
    //连接数据库
    if(database_connect()==false)
    {
        LOG_PERROR("database_connect");        
        exit(1);
    }
    //初始化数据库  
    const char* g="CREATE TABLE IF NOT EXISTS chat_group(\
    groupname VARCHAR(128),\
    groupowner VARCHAR(128),\
    groupnumber VARCHAR(4096)\
    )charset utf8mb4";
    if(!exec_update(g))
    {
        LOG_ERROR("exec_update");
        return false;
    }
    const char* q="CREATE TABLE IF NOT EXISTS chat_user(\
    username VARCHAR(128),\
    password VARCHAR(128),\
    friendlist VARCHAR(4096),\
    grouplist VARCHAR(4096)\
    )charset utf8mb4";
    if(!exec_update(q))
    {
        LOG_ERROR("exec_update");
        return false;
    }
    //断开连接
    database_disconnect();
    return true;
}

bool DataBase::database_user_is_exist(std::string usr)
{
    char sql[256];
    sprintf(sql,"SELECT * FROM chat_user WHERE username ='%s';",usr.c_str());
    MYSQL_ROW row;
    return exec_query_and_fetch_row(sql, row);
}

void DataBase::database_insert_user_info(Json::Value& v)
{
    std::string username=v["username"].asString();
    std::string password=v["password"].asString();
    char sql[256]={0};
    sprintf(sql,"INSERT INTO chat_user(username,password) VALUES('%s','%s');",username.c_str(),password.c_str());
    exec_update(sql);
}

bool DataBase::database_password_correct(Json::Value& v)
{
    char sql[256]={0};
    sprintf(sql,"SELECT password FROM chat_user WHERE username='%s';",v["username"].asCString());
    MYSQL_ROW row;
    if(!exec_query_and_fetch_row(sql, row))
    {
        LOG_ERROR("exec_query_and_fetch_row");
        return false;
    }
    // 密码不正确
    if(strcmp(row[0],v["password"].asCString()))    return false;
    //密码正确
    else return true;
}

bool DataBase::database_get_friend_group(const Json::Value& v,std::string& friList,std::string& groList)
{
    char sql[256]={0};
    sprintf(sql,"SELECT COALESCE(friendlist,''),COALESCE(grouplist,'') \
    FROM chat_user WHERE username='%s';",v["username"].asCString());
    MYSQL_ROW row;
    if(!exec_query_and_fetch_row(sql, row))
    {
        LOG_ERROR("exec_query_and_fetch_row");
        return false;
    }
    //将好友列表和群组列表赋值
    friList=std::string(row[0]);
    groList=std::string(row[1]);
    return true;
}

void DataBase::database_update_friendlist(const std::string& u,const std::string& f)
{
    std::string friendlist;
    //查询数据库中的u的好友列表
    char sql[256]={0};
    sprintf(sql,"SELECT COALESCE(friendlist,'') FROM chat_user WHERE username='%s';",u.c_str());
    MYSQL_ROW row;
    if(!exec_query_and_fetch_row(sql,row))
    {
        LOG_ERROR("exec_query_and_fetch_row");
        return;
    }
    friendlist=std::string(row[0]);
    //如果为空串
    if(friendlist.empty())
    {
        friendlist.append(f);
    }
    else
    {
        friendlist.append("|");
        friendlist.append(f);
    }

    //更新好友列表
    memset(sql,0,sizeof(sql));
    sprintf(sql,"UPDATE chat_user SET friendlist='%s' WHERE username='%s';",friendlist.c_str(),u.c_str());
    if(!exec_update(sql))
    {
        LOG_ERROR("exec_update");
        return;
    }
}

void DataBase::database_add_friend(const Json::Value& v)
{
    std::string username=v["username"].asString();
    std::string friendname=v["friend"].asString();
    database_update_friendlist(username,friendname);
    database_update_friendlist(friendname,username);
}


