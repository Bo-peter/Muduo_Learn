#pragma once

#include <functional>

#include "noncopyable.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;

class Acceptor : noncopyable
{
public:
    typedef std::function<void(int sockfd,const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop,const InetAddress& listenAddr,bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ =cb;
    }
    bool listenning() const {return listenning_;}
    void listen();
private:
    void handleRead();

    EventLoop* loop_;       //Acceptor 用的就是用户定义的那个baseLoop，也称作mainLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
    int idleFd_;
};
