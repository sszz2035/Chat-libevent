#include"chat_thread.h"

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
        perror("event_new error");
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

std::thread::id ChatThread::get_id()
{
    return _id;
}

struct event_base *ChatThread::thread_get_base()
{
    return base;
}


void ChatThread::thread_readcb(struct bufferevent *bev, void *ctx)
{   
    char buf[1024];
    memset(buf,0,sizeof(buf));
    //读取数据
    bufferevent_read(bev,buf,sizeof(buf));
    //打印数据
    std::cout<<buf<<std::endl;
}   

void ChatThread::thread_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    
}