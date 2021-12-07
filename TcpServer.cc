#include "TcpServer.h"
#include "Logger.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include <assert.h>
#include <atomic>
#include <strings.h>

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d loop == nullptr", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     Option option)
    : loop_(CheckLoopNotNull(loop))
    , ipPort_(listenAddr.toIpPort())
    , name_(nameArg)
    , acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
    , threadPool_(new EventLoopThreadPool(loop, nameArg))
    , connectionCallback_()
    , messageCallback_()
    , nextConnId_(1)
{
    //当有新用户连接时，会执行TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
}
TcpServer::~TcpServer()
{
    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        // conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if (started_++ == 0)    //防止一个TcpServer对象被调用多次
    {
        threadPool_->start(threadInitCallback_);
        // 启动底层的loop线程池
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

sockaddr_in getLoaclAddr(int sockfd)
{
    sockaddr_in localaddr;
    socklen_t addrlen = sizeof localaddr;
    bzero(&localaddr, addrlen);
    if (::getsockname(sockfd, (sockaddr *)&localaddr, &addrlen) < 0)
    {
        LOG_ERRRO("sockets::getLocalAddr error");
    }
    return localaddr;
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
    InetAddress localAddr(getLoaclAddr(sockfd));

    // TcpConnectionPtr conn(new TcpConnection(ioLoop,
    //                                         connName,
    //                                         sockfd,
    //                                         localAddr,
    //                                         peerAddr));
    // connections_[connName] = conn;
    // conn->setConnectionCallback(connectionCallback_);
    // conn->setMessageCallback(messageCallback_);
    // conn->setWriteCompleteCallback(writeCompleteCallback_);
    // conn->setCloseCallback(
    //     std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
    // ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    // LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s",name_.size(),conn->name());
    // size_t n = connections_.erase(conn->name());
    // void(n);
    // EventLoop* ioLoop = conn->getLoop();
    // ioLoop->queueInLoop(
    //     std::bind(&TcpConnection::connectDestroyed, conn));
}