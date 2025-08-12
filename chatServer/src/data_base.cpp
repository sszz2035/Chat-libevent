#include"data_base.h"
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
        perror("mysql_init error");
        return false;
    }
    //连接mysql
    if(mysql_real_connect(mysql,"localhost","root","5201314Zqzq!","chat_database",0,NULL,0)==NULL)
    {
        perror("mysql_real_connect error");
        return false;
    }

    //设置编码格式为utf8mb4
    if(mysql_query(mysql,"SET NAMES utf8mb4")!=0)
    {
        perror("mysql_query error");
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
        perror("query error");
        return -1;
    }
    //存储结果
    MYSQL_RES* res=mysql_store_result(mysql);
    if(res==NULL)
    {
        perror("mysql_store_result error");
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

