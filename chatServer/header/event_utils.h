#ifndef EVENTUTILS_H
#define EVENTUTILS_H
#include <event.h>
//为event_base* 指针定制的删除器
struct EventBaseDeleter
{
    void operator()(struct event_base* base)const
    {
        if(base)
        {
            event_base_free(base);
        }
    }
};

#endif