#include "Channel.h"
#include <sys/epoll.h>
#include "EventLoop.h"
#include "Logger.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

Channel::~Channel() {}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

/**
 * @brief 
 * 当改变channel所表示fd的events事件后，update负责在poller里面更改fd相应的事件epoll_ctl
 * EventLoop=> ChannelList  Poller
 */
void  Channel::update()
{
    //通过channel所属的EventLoop,调用poller的相应的发放，注册fd的events事件
    loop_->updateChannel(this);
}

//在Channel 所属的EventLoop中,把当前的channel删除掉
void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    std::shared_ptr<void> guard;
    if(tied_)
    {
        guard =tie_.lock();
        if(guard)
        {
            handlerEventWithGuard(receiveTime);
        }
    }
    else
    {
        handlerEventWithGuard(receiveTime);
    }
}

//根据poller通知的具体事件，调用对应的操作
void Channel::handlerEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d",revents_);
    if((revents_&EPOLLHUP)&&!(revents_&EPOLLIN))
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }
    if(revents_ & EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }
    if(revents_ & EPOLLIN)
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }
    if(revents_&EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}