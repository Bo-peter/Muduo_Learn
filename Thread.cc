#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>
#include <iostream>

std::atomic_int Thread::numCreated_ {0};

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{
}
Thread::~Thread()
{
    if(started_ && !joined_)
    {
        thread_->detach();
    }
}

//一个Thread对象记录就是一个新现成的详细信息
void Thread::start()   
{
    started_= true;
    sem_t sem;
    sem_init(&sem,0,0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        //开启一个新线程，专门执行该线程函数
        func_(); 
    }));

    //这里必须等待获取上面新创建的线程的tid值
    sem_wait(&sem);

}
void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf,sizeof buf,"Thread%d",num);
        name_ =buf;
    }
}
