#pragma once
#include <string>
#include <memory>
#include <atomic>

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

class EventLoop;
class Channel;
class Socket;

/**
 * TcpServer --> Acceptor ==> 有一个新用户连接，通过accpet函数拿到connfd
 * --> TcpConnecton 这是回调  --》Channel -->Poller ---> Channel的回调操作
 * 
 */
class TcpConnection:noncopyable,
                        public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop()const {return loop_;}
    const std::string& name()const {return name_;}
    const InetAddress& localAddress()const {return localAddr_;}
    const InetAddress& peerAddress()const {return peerAddr_;}

    bool connected() {return state_ == kConnected;}

    void send(const void* message,int len);
    // void send(const std::stringPiece& message);
    void send(Buffer* message);
    //关闭连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    {connectionCallback_ = cb;}

    void setMessageCallback(const MessageCallback& cb)
    {messageCallback_  = cb;}

    void setWriteCompleteCallback(const WriteCompleteCallback&cb)
    {writeCompleteCallback_= cb;}

    void setCloseCallback(const CloseCallback& cb)
    {closeCallback_ = cb;}

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb)
    {highWaterMarkCallback_ = cb;}
    //连接建立
    void connectEstablished();
    //连接销毁
    void connectDestroyed();

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message,size_t len);
    void shutdownInLoop();
private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    EventLoop* loop_;   //这里绝对不是baseLoop ,因为TcpConnection都是在subLoop中管理的
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
    //这里和Acceptor 类似 Acceptor => mainLoop  TcpConnection => subLoop
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;
    CloseCallback closeCallback_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;

};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;