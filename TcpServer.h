#pragma once

/**
 * @file TcpServer.h
 * @author your name (you@domain.com)
 * @brief 用户使用muduo 编写服务器程序
 * @version 0.1
 * @date 2021-12-06
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <functional>
#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>

#include "noncopyable.h"
#include "Callbacks.h"
#include "InetAddress.h"

class Acceptor;
class EventLoop;
class EventLoopThreadPool;
//对外使用的类
class TcpServer : noncopyable
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    enum Option //是否对端口可重用
    {
        kNoReusePotr,
        kReusePort,
    };

    TcpServer(EventLoop *loop,
              const InetAddress &listenAddr,
              const std::string &nameArg,
              Option option = kNoReusePotr);
    ~TcpServer();

    void setThreadNum(int numThreads);

    std::shared_ptr<EventLoopThreadPool> threadPool()
    {
        return threadPool_;
    }

    void start();

    void setThreadInitCallback(const ThreadInitCallback &cb)
    {
        threadInitCallback_ = cb;
    }
    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback &cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);

    void removeConnection(const TcpConnectionPtr &conn);

    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    typedef std::unordered_map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop *loop_; // baseLoop 用户定义的Loop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;              //运行的mainLoop ，监听新的连接事件
    std::shared_ptr<EventLoopThreadPool> threadPool_; // one loop per thread

    ConnectionCallback connectionCallback_;       //有新连接的回调
    MessageCallback messageCallback_;             //有读写消息的回调
    WriteCompleteCallback writeCompleteCallback_; //消息发完以后的回调

    ThreadInitCallback threadInitCallback_; // loop线程初始化的回调

    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_; //保存所有的连接
};