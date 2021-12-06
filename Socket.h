#pragma once


#include "noncopyable.h"

class InetAddress;

//封装socket fd
class Socket
{
public:
    explicit Socket(int sockfd)
    :sockfd_(sockfd)
    {}
    ~Socket();

    int fd()const {return sockfd_;}
    void bindAddress(const InetAddress &localaddr);
    void listen();
    int accept(InetAddress* peeraddr);

    void shutdownWrite();
    void setTcpNoDelay(bool on);
    void setResuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    int sockfd_;
};