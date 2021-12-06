#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <memory>

#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

//防止一个线程创建多个EventLoop __thread <thread_local>机制
__thread EventLoop *t_loopInThisThread = nullptr;

//定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

//创建wakeupfd,用来notify唤醒subReactor处理信赖的Channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd error:%d\n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    //设置weakup 的事件类型和发生事件后的回调操作
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this));
    //每一个eventloop都监听wakeupchannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping", this);
    while (!quit_)
    {
        activeChannels_.clear();
        //监听两类fd  一种是client 的fd,一种wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel *channel : activeChannels_)
        {
            //Poller监听哪些channel发生事件了，然后上报给Eventloop,通知channel处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }
        //执行当前EventLoop 事件循环需要处理的回调操作
        /**
         * IO 线程 mainLoop accept fd(使用channel 打包fd 发送给 subloop) 
         * mainLoop 事先注册一个回调cb (需要subloop来执行)   wakeup subloop后，执行之前mainloop注册的回调
         */
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping", this);
    looping_ = false;
}
/**
 * 退出事件循环
 * 1.loop在自己的线程中调用quit
 * 2.在非loop的线程中，调用loop的quit
 */
void EventLoop::quit()
{
    quit_ = true;
    //如果是在其他线程中，调用的quit 在一个subloop中，调用了mainLoop的quit
    if (!isInLoopThread())
    {
        wakeup();
    }
}

//在当前loop中执行
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread()) //在当前的loop线程中，执行cb
    {
        cb();
    }
    else //在非当前loop线程中执行cb，就需要唤醒loop所在的线程 执行cb
    {
        queueInLoop(std::move(cb));
    }
}
//把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    //唤醒相应的，需要执行上面回调操作得到loop的线程
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();//唤醒loop所在的线程
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERRRO("EventLoop::handleRead() reads %lu bytes instead of 8", n);
    }
}

//用来唤醒loop所在的线程的
//向wakeupfd_写一个数据
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_,&one,sizeof one);
    if(n != sizeof one)
    {
        LOG_ERRRO("EventLoop :: wakeup() writes %lu bytes instead of 8 \n",n);
    }
}

//EventLoop的方法 =》Poller的方法
void  EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}
void  EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}
bool  EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void  EventLoop::doPendingFunctors() //执行回调
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor& functor:functors)
    {
        functor();      //执行当前loop需要执行的回调
    }
    callingPendingFunctors_ = false;
}