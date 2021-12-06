#include <stdlib.h>

#include "Poller.h"
#include "EPollPoller.h"

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
    {
        //生成poll的实例
        //return new PollPoller(loop);
        return nullptr;
    }
    else
    {
        //生成epool的实例
        return new EPollPoller(loop);
    }
}