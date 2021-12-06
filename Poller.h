#pragma once
#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;
class Poller : noncopyable
{
public:
    typedef std::vector<Channel *> ChannelList;

    Poller(EventLoop *loop);
    virtual ~Poller();
    //给所有 IO 复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    //判断参数Channel是否在当前Poller当中
    bool hasChannel(Channel* channel) const;

    //EventLoop 可以通过该接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop* Loop);
protected:
    //map的key： sockfd
    //value: sockfd 所属的Channl通道类型
    typedef std::unordered_map<int, Channel *> ChannelMap;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; //定义Poller所属的时间循环EvenLoop
};