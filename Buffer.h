#pragma once
#include <iostream>
#include <vector>
#include <string>

//网络库底层的缓冲器类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
    }
    void swap(Buffer &rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }
    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    //返回缓冲区中可读数据的起始地址
    const char *peek() const
    {
        return begin() + readerIndex_;
    }

    // onMessage string<-Buffer
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len; //应用只读取了可读缓冲区的一部分
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    //把onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        //读取缓冲区中可读的信息
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }
    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    void append(const char* data,size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data,data+len,beginWrite());
        writerIndex_+=len;
    }
    char* beginWrite()
    {return begin()+writerIndex_;}
    const char* beginWrite()const
    {return begin()+writerIndex_;}

    //从fd上读取数据
    ssize_t readFd(int fd,int* saveErrno);
private:
    char *begin() //数组首元素的地址，数组的起始地址
    {
        return &*buffer_.begin();
    }
    const char *begin() const
    {
        return &*buffer_.begin();
    }
    void makeSpace(size_t len)
    {
        /**
         |  kCheapPrepend |  reader |    writer  |
         |                | 读取之后空出来的 |  reader |    writer  |
         
         */
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};