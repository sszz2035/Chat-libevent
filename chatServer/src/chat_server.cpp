#include"chat_server.h"
#include"log.h"
ChatServer::ChatServer()
{
    //初始化事件集合
    this->base=event_base_new();
    //初始化数据库对象
    db=new DataBase();
    //初始化数据结构对象
    info=new ChatInfo();
    //初始化数据表
    if(!db->database_init_table())
    {
        LOG_PERROR("database_init_table");        
        exit(1);
    }
    //初始化群消息:把群信息从数据库读出来，放入map
    server_update_group_info();
    
    //初始化线程池
    thread_num=POOL_MAX_THREAD_SZ;
    cur_thread=0;
    pool=new ChatThread[thread_num];
    for(int i=0;i<thread_num;i++)
    {
        pool[i].start(info,db);
    }
}

ChatServer::~ChatServer()
{
    if(db)  delete db;
    if(info) delete info;
}

/*创建监听对象*/
void ChatServer::listen(const char* ip,int port)
{
    //创建信息结构体
    struct sockaddr_in client_info;
    client_info.sin_family=AF_INET;
    client_info.sin_addr.s_addr=inet_addr(ip);
    client_info.sin_port=htons(port);
    //进入监听状态
    struct evconnlistener* listener=evconnlistener_new_bind(base,listener_cb,this,LEV_OPT_CLOSE_ON_FREE| LEV_OPT_REUSEABLE
    ,5,(struct sockaddr*)&client_info,sizeof(client_info));

    if(listener==NULL)
    {
        LOG_PERROR("evconnlistener_new_bind");        
        exit(1);
    }

    //监听集合
    event_base_dispatch(base);

    //释放对象
    evconnlistener_free(listener);
    event_base_free(base);
}

//listen的回调函数
void ChatServer::listener_cb(struct evconnlistener* listener,
evutil_socket_t sock,struct sockaddr* addr,int len,void* ptr)
{
    struct sockaddr_in* client_info=reinterpret_cast<sockaddr_in*>(addr);
    ChatServer* ser=(ChatServer*)ptr;
    //打印连接消息
    std::cout<<"[connected] "
    <<"Ip:"<<inet_ntoa(client_info->sin_addr)
    <<" Port: "<<ntohs(client_info->sin_port)<<std::endl;
    //将连接添加到线程池
    ser->server_alloc_event(sock);
}

//更新群消息列表
void ChatServer::server_update_group_info()
{
    //连接数据库
    if(db->database_connect()==false)
    {
        LOG_PERROR("database_connect");        
        exit(1);
    }
    //获取群列表信息
    std::string groupinfo[GROUP_MAX_SZ];//最多1024个群
    int num=db->database_get_group_info(groupinfo);
    
    //更新info中的群信息
    info->list_update_group(groupinfo,num);
    info->list_print_group();
    
    //断开数据库
    db->database_disconnect();
}

//将连接添加到线程池
void ChatServer::server_alloc_event(int fd)
{
    //要操作的线程
    struct ChatThread *t=&pool[cur_thread];
    struct event_base *t_base= t->thread_get_base();
    cur_thread=(cur_thread+1)%thread_num;
    std::cout<<"--- 线程"<<t->thread_get_id()<<"已分配任务"<<std::endl;
    // 创建套接字缓冲事件
    struct bufferevent* evt=bufferevent_socket_new(t_base,fd,BEV_OPT_CLOSE_ON_FREE);
    if(evt==NULL)
    {
        LOG_PERROR("bufferevent_socket_new");        
        return ;
    }
    //设置回调函数
    bufferevent_setcb(evt,ChatThread::thread_readcb,NULL,ChatThread::thread_event_cb,t);
    //启用读事件
    bufferevent_enable(evt,EV_READ);
}
