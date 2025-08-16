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
    //初始化数据库句柄
    mysql=mysql_init(NULL);
    if(mysql==NULL)
    {
        LOG_PERROR("mysql_init_error");
        return false;
    }
    //连接mysql
    if(mysql_real_connect(mysql,"localhost","root","5201314Zqzq!","chat_database",0,NULL,0)==NULL)
    {
        LOG_PERROR("mysql_real_connect");        
        return false;
    }

    //设置编码格式为utf8mb4
    if(mysql_query(mysql,"SET NAMES utf8mb4")!=0)
    {
        LOG_PERROR("mysql_query");        
        return false;
    }
    return true;
}

void DataBase::database_disconnect()
{
    mysql_close(mysql);
}

int DataBase::data_base_get_group_info(std::string* g)
{
    //查询所有群信息
    if(mysql_query(mysql,"SELECT * FROM chat_group;"))
    {
        LOG_PERROR("mysql_query");        
        return -1;
    }
    //存储结果
    MYSQL_RES* res=mysql_store_result(mysql);
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
    if(mysql_query(mysql,g))
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
    if(mysql_query(mysql,q))
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
    char sql[256];
    sprintf(sql,"SELECT * FROM chat_user WHERE username ='%s';",usr.c_str());
    //查询是否有此用户
    if(mysql_query(mysql,sql)!=0)
    {
        LOG_PERROR("mysql_query");        
        return true;
    }
    //储存查询结果
    MYSQL_RES *res=mysql_store_result(mysql);
    if(res==NULL)
    {
        LOG_PERROR("mysql_store_result");       
        return true;
    }
    //判断查询结果
    MYSQL_ROW row=mysql_fetch_row(res);
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
    std::string username=v["username"].asString();
    std::string password=v["password"].asString();
    char sql[256]={0};
    sprintf(sql,"INSERT INTO chat_user(username,password) VALUES('%s','%s');",username.c_str(),password.c_str());
    if(mysql_query(mysql,sql))
    {
        LOG_PERROR("mysql_query");       
        return ;
    }
}