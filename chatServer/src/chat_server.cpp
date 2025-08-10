#include"chat_server.h"
ChatServer::ChatServer()
{
    //初始化事件集合
    this->base=event_base_new();
}

ChatServer::~ChatServer()
{

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
    struct evconnlistener* listener=evconnlistener_new_bind(base,listener_cb,NULL,LEV_OPT_CLOSE_ON_FREE
    ,5,(struct sockaddr*)&client_info,sizeof(client_info));

    if(listener==NULL)
    {
        perror("evconnlistener_new_bind error");
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
    //打印连接消息
    std::cout<<"[connected] "<<"Ip:"<<inet_ntoa(client_info->sin_addr)<<" Port: "<<ntohs(client_info->sin_port)<<std::endl;
    //处理连接

    //将连接添加到线程池
}

