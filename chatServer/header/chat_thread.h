#ifndef CHATTHREAD_H
#define CHATTHREAD_H
#include "chat_info.h"
#include "data_base.h"
#include <thread>
#include <event.h>
class ChatThread
{
public:
    /**
     * @brief 构造函数，创建新线程，并让其开始工作
     * @details 调用了worker函数
    */
    ChatThread();
    ~ChatThread();

    /**
     * @brief 给info和db初始化
     * @param info 链表指针，指向服务器的链表
     * @param db 数据库指针，指向服务器的数据库
     */
    void start(ChatInfo *info, DataBase *db);

    /**
     * @brief 工作函数，用于线程池工作
     * @param 要操作的线程
     */
    static void worker(ChatThread* t);

    /**
     * @brief 获取线程号
     * @return 返回线程id
     */
    std::thread::id get_id();

private:
    // 线程
    std::thread *_thread;
    // 线程号
    std::thread::id _id;
    // 事件集合
    struct event_base *base;
    // 数据库对象
    DataBase *db;
    // 数据结构对象
    ChatInfo *info;

    /**
     * @brief 用于worker函数内部，是工作的具体行为
     */
    void run();

    /**
     * @brief 定时器的回调函数
     */
    static void timeout_cb(evutil_socket_t fd, short event, void *arg);
};
#endif