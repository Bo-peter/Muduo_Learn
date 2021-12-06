
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include "Acceptor.h"
#include "Logger.h"
#include "EventLoop.h"
#include "InetAddress.h"

static int createNonblockingOrDie(sa_family_t family)
{
    int sockfd = ::socket(family,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
    if(sockfd<0)
    {
        LOG_FATAL("%s:%s:%d create socket error ,errno : %d",__FILE__,__FUNCTION__,__LINE__,errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    :loop_(loop)
    ,acceptSocket_(createNonblockingOrDie(AF_INET))
    ,acceptChannel_(loop,acceptSocket_.fd())
    ,listenning_(false)
    , idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    acceptSocket_.setResuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

//listenfd 有事件发生了，有新用户连接了
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd>= 0)
    {
        if(newConnectionCallback_)
        {
            newConnectionCallback_(connfd,peerAddr);//轮询找到subLoop,唤醒，分发当前的新客户端的Channel
        }
        else
        {
            ::close(connfd);
            LOG_ERRRO("%s:%s:%d accept  error ,errno : %d",__FILE__,__FUNCTION__,__LINE__,errno);
            if(errno == EMFILE)
            {
            LOG_ERRRO("%s:%s:%d sockfd reached limit ",__FILE__,__FUNCTION__,__LINE__);
            }
        }
    }
}