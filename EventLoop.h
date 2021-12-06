#pragma once

#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include <functional>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

//事件循环 主要包含了两个大模块 Channel Poller(epoll)的抽象
class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    void loop(); //开启事件循环
    void quit(); //退出事件循环

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    //在当前loop中执行
    void runInLoop(Functor cb);
    //把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);
    //用来唤醒loop所在的线程的
    void wakeup();

    //EventLoop的方法 =》Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);
    //判断EventLoop对象是否处在自己的线程里面
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void handleRead();        //waked up
    void doPendingFunctors(); //执行回调

    void printActiveChannels() const;

    typedef std::vector<Channel *> ChannelList;
    std::atomic_bool looping_; //原子操作的布尔值 通过CAS实现
    std::atomic_bool quit_;    //标致退出loop循环

    const pid_t threadId_; //记录当前 loop 所在线程的id

    Timestamp pollReturnTime_; //poller返回发生事件的channels的事件点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_; //主要作用,当mainLoop 获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel *currentActiveChannel_;

    std::atomic_bool callingPendingFunctors_; //标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;    //存储loop需要执行的所有回调操作
    std::mutex mutex_;                        //互斥锁，用来保护上面vector容器的线程安全操作
};