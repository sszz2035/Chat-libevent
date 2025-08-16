#include"chat_thread.h"
#include"log.h"
ChatThread::ChatThread()
{
    // 创建事件集合
    base=event_base_new();
    //创建新线程
    _thread=new std::thread(worker,this);
    //将id赋值为线程id号
    _id=_thread->get_id();
}

ChatThread::~ChatThread()
{
    if(_thread) delete _thread;
}

void ChatThread::start(ChatInfo* info,DataBase*db)
{
    this->info=info;
    this->db=db;
}

void ChatThread::worker(ChatThread* t)
{
    t->run();
}

void ChatThread::run()
{
    //创建一个定时器事件
    struct event* timeout=event_new(base,-1,EV_PERSIST,timeout_cb,this);
    if (!timeout) 
    {
        LOG_PERROR("event_new");        
        return;
    }

    struct timeval tv;
    //给tv变量清0
    evutil_timerclear(&tv);
    tv.tv_sec=3;
    //将事件添加到base中
    event_add(timeout,&tv);
    std::cout<<"--- thread " <<_id<<" start working ..."<<std::endl;
    //开始执行事件
    event_base_dispatch(base);

    //回收事件集合
    event_base_free(base);
}

void ChatThread::timeout_cb(evutil_socket_t fd,short event,void* arg)
{
    ChatThread* t=(ChatThread*)arg;
    // std::cout<<"thread"<<t->get_id()<<" is listening"<<std::endl;
}

std::thread::id ChatThread::thread_get_id()
{
    return _id;
}

struct event_base *ChatThread::thread_get_base()
{
    return base;
}


void ChatThread::thread_readcb(struct bufferevent *bev, void *ctx)
{   
    ChatThread* t=(ChatThread*)ctx;
    char buf[1024];
    memset(buf,0,sizeof(buf));
    if(!t->thread_read_data(bev,buf))
    {
        LOG_PERROR("thread_read_data");        
        return;
    }
    std::cout<<"--thread "<<t->thread_get_id()<<" receive data "<<buf<<std::endl;

    //解析json数据
    Json::Reader reader;//json 解析对象
    Json::Value  val; //json存储的容器 类似于map

    if(!reader.parse(buf,val))
    {
        LOG_PERROR("parse");        
        return;
    }
    //处理注册事件
    if(val["cmd"]=="register")
    {
        // std::cout<<"客户端注册"<<std::endl;
        t->thread_register(bev,val);
        return;
    }

}   

void ChatThread::thread_event_cb(struct bufferevent *bev, short events, void *ctx)
{   

}

bool ChatThread::thread_read_data(struct bufferevent* bev,char *s)
{
    //要读取的长度
    int sz=0;
    //先读取长度，放入sz中
    if(bufferevent_read(bev,&sz,4)!=4)
    {
        LOG_PERROR("bufferevent_read");       
        return false;
    }
    size_t count=0;
    char buf[1024]={0};
    while(1)
    {
        //count加上读取到的字符个数
        count+=bufferevent_read(bev,buf,1024);
        strcat(s,buf);
        memset(buf,0,sizeof(buf));
        //读取到的字符个数>=要读取的字符个数
        if(count>=sz)   break;
    }
    return true;
}

void ChatThread::thread_register(struct bufferevent* bev,Json::Value& v)
{
    //连接数据库
    if(db->database_connect()==false)
    {   
        LOG_PERROR("database_connect");        
        return;
    }
    //判断用户是否已存在

    //用户已存在
    if(db->database_user_is_exist(v["username"].asString()))
    {
        Json::Value val;
        val["cmd"]="register_reply";
        val["result"]="user_exist";
        //发送回应给客户端
        thread_write_data(bev,val);
    }

    //用户不存在
    else
    {   
        //将用户添加到数据库中
        db->database_insert_user_info(v);
        //发送回应给客户端        
        Json::Value val;
        val["cmd"]="register_reply";
        val["result"]="success";
        thread_write_data(bev,val);
    }

    //断开数据库
    db->database_disconnect();
}

void ChatThread::thread_write_data(struct bufferevent* bev,Json::Value& v)
{
    //封装json数据为字符串
    Json::FastWriter writer;
    std::string val=writer.write(v);
    int len=val.size();
    char wbuf[1024]={0};
    memcpy(wbuf,&len,4);
    memcpy(wbuf+4,val.c_str(),len);
    //将数据写入输出缓冲区中
    bufferevent_write(bev,wbuf,len+4);
}