#include <minimuduo/TcpServer.h>
#include <minimuduo/Logger.h>
#include <minimuduo/EventLoop.h>

#include <minimuduo/TcpConnection.h>
#include <string>
#include <iostream>
#include <functional>
class EchoServer
{
public:
    EchoServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const std::string &nameArg)
        : server_(loop, listenAddr, nameArg), loop_(loop)
    {
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

        server_.setThreadNum(3);

    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("Connection UP : %s\n",conn->peerAddress().toIpPort().c_str());
            std::cout << "一个用户已连接" << std::endl;
        }
        else
        {
             LOG_INFO("Connection Down : %s\n",conn->peerAddress().toIpPort().c_str());
            std::cout << "一个用户已断开" << std::endl;
        }
    }

    void onMessage(const TcpConnectionPtr &conn,
                   Buffer * buffer,
                   Timestamp time)
    {
        std::string buf = buffer->retrieveAllAsString();
        conn->send(buf);
        conn->shutdown();
    }
    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8800);
    EchoServer server(&loop,addr,"Demo");
    server.start();
    loop.loop();
    return 0;
}