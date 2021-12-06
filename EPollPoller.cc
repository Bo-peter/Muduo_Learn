
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

/**
 * epoll 的使用
 * epoll_create
 * epoll_ctl   add/mov/del
 * epoll_wait
 */
const int kNew = -1; //一个channel 没有添加到poller
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) //vector<epoll_event>
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}
EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

//重写基类Poller的抽象方法
Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_INFO("func = %s => fd total count:%lu\n", __FUNCTION__,channels_.size());
    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents>0)
    {
        LOG_INFO("%d events hanppened\n",numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents == events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_INFO("%s timeout \n",__FUNCTION__);
    }
    else
    {
        if(savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_ERRRO("EPollPoller::poll() err");
        }
    }
    return now;
}

//channel update remove =>Eventloop upadate remove
void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("fd = %d,events = %d, index = %d \n",
             channel->fd(),
             channel->events(), index);

    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        //更新已有的事件
        int fd = channel->fd();
        (void)fd; //?
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    int index = channel->index();
    size_t n = channels_.erase(fd);
    (void)n;
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

//填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}
//更新channel 通道
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;

    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        //打印日志
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERRRO("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}
