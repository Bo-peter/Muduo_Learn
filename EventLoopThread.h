#pragma once
#include <functional>
#include "noncopyable.h"
#include "Thread.h"
#include <mutex>
#include <condition_variable>

class EventLoop;
class EventLoopThread : noncopyable
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback&cb = ThreadInitCallback(),
    const std::string &name =std::string());
    ~EventLoopThread();

    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};