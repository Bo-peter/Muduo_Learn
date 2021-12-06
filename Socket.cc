#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr)
{
    if(0 != bind(sockfd_,localaddr.getSockAddr(),sizeof(sockaddr_in)))
    {
        LOG_FATAL("bind sockfd:%d fail \n",sockfd_);
    }
}
void Socket::listen()
{
    if(-1 == ::listen(sockfd_,1024))
    {
        LOG_FATAL("listen sockfd:%d fail \n",sockfd_);
    }
}
int Socket::accept(InetAddress *peeraddr)
{
    struct sockaddr_in addr;
    int len = sizeof(sockaddr_in);
    bzero(&addr,len);
    int connfd = ::accept(sockfd_,(sockaddr*)&addr,(socklen_t*)&len);
    if(connfd >= 0)
    {
        peeraddr ->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_,SHUT_WR)<0)
    {
        LOG_ERRRO("shutdown sockfd:%d fail \n",sockfd_);
    }
}
void Socket::setTcpNoDelay(bool on)
{
    int optval =on?1:0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof(int));
}
void Socket::setResuseAddr(bool on)
{
    int optval =on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int));
}
void Socket::setReusePort(bool on)
{
    int optval =on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&optval,sizeof(int));
}
void Socket::setKeepAlive(bool on)
{
    int optval =on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof(int));
}