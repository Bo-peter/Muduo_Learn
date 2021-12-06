#pragma once

#include <functional>
#include <string>
#include <thread>
#include <unistd.h>
#include <atomic>
#include <memory>

#include "noncopyable.h"


class Thread : noncopyable
{
public:
    typedef std::function<void()> ThreadFunc;

    explicit Thread(ThreadFunc,const std::string& name = std::string());
    ~Thread();

    void start();
    void join();
    bool started() const {return started_;}

    pid_t tid() const{return tid_;}
    const std::string& name()const{return name_;}
    static int32_t numCreated(){return numCreated_;}
private:
    void setDefaultName();

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;

    static std::atomic_int numCreated_;
};