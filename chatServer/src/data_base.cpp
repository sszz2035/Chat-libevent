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
    if(mysql_query(mysql.get(),g))
    {
        LOG_PERROR("mysql_query");        
        return false;
    }
    const char* q="CREATE TABLE IF NOT EXISTS chat_user(\
    username VARCHAR(128),\
    password VARCHAR(128),\
    friendlist VARCHAR(4096),\
    grouplist VARCHAR(4096)\
    )charset utf8mb4";
    if(mysql_query(mysql.get(),q))
    {
        LOG_PERROR("mysql_query");      
        return false;
    }
    //断开连接
    database_disconnect();
    return true;
}

bool DataBase::database_user_is_exist(std::string usr)
{
    //上读锁
    std::shared_lock<std::shared_mutex> lock(mutex_);

    char sql[256];
    sprintf(sql,"SELECT * FROM chat_user WHERE username ='%s';",usr.c_str());
    //查询是否有此用户
    if(mysql_query(mysql.get(),sql)!=0)
    {
        LOG_PERROR("mysql_query");        
        return false;
    }
    //储存查询结果
    MYSQL_RES *res=mysql_store_result(mysql.get());
    if(res==NULL)
    {
        LOG_PERROR("mysql_store_result");       
        return false;
    }
    //判断查询结果
    MYSQL_ROW row=mysql_fetch_row(res);
    
    mysql_free_result(res);

    if(row==NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void DataBase::database_insert_user_info(Json::Value& v)
{
    //上写锁
    std::unique_lock<std::shared_mutex>lock(mutex_);

    std::string username=v["username"].asString();
    std::string password=v["password"].asString();
    char sql[256]={0};
    sprintf(sql,"INSERT INTO chat_user(username,password) VALUES('%s','%s');",username.c_str(),password.c_str());
    if(mysql_query(mysql.get(),sql))
    {
        LOG_PERROR("mysql_query");       
        return ;
    }
}

bool DataBase::database_password_correct(Json::Value& v)
{
    //上读锁
    std::shared_lock<std::shared_mutex> lock(mutex_);
    //查询用户密码
    char sql[256]={0};
    sprintf(sql,"SELECT password FROM chat_user WHERE username='%s';",v["username"].asCString());
    if(mysql_query(mysql.get(),sql))
    {
        LOG_PERROR("mysql query");
        return false;
    }
    MYSQL_RES* res=mysql_store_result(mysql.get());
    if(res==NULL)
    {
        LOG_PERROR("mysql_store_result");       
        return false;
    }
    //判断查询结果
    MYSQL_ROW row=mysql_fetch_row(res);
    if(row==NULL)
    {
        LOG_PERROR("mysql_fetch_row");
        return false;
    }

    //释放结果
    mysql_free_result(res);

    // 密码不正确
    if(strcmp(row[0],v["password"].asCString()))    return false;
    //密码正确
    else return true;
}

bool DataBase::database_get_friend_group(const Json::Value& v,std::string& friList,std::string& groList)
{
    //添加读锁(必要的，保护mysql变量不受其他影响)
    std::shared_lock<std::shared_mutex> lock(mutex_);

    //查询用户的好友列表和群组列表
    char sql[256]={0};
    sprintf(sql,"SELECT COALESCE(friendlist,''),COALESCE(grouplist,'') \
    FROM chat_user WHERE username='%s';",v["username"].asCString());
    if(mysql_query(mysql.get(),sql))
    {
        LOG_PERROR("mysql_query");
        return false;
    }
    MYSQL_RES* res=mysql_store_result(mysql.get());
    if(res==NULL)
    {
        LOG_PERROR("mysql_store_result");
        return false;
    }
    MYSQL_ROW row =mysql_fetch_row(res);
    if(row==NULL)
    {
        LOG_PERROR("mysql_fetch_row");
        return false;
    }
    //将好友列表和群组列表赋值
    friList=std::string(row[0]);
    groList=std::string(row[1]);
    //释放结果
    mysql_free_result(res);

    return true;
}
