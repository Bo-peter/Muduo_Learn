#pragma once

/**
 * @brief 
 * 继承noncopyable 之后 ，其失去拷贝构造 和赋值构造
 */
class noncopyable
{
    public:
    noncopyable(const noncopyable&) =delete;
    noncopyable& operator=(const noncopyable&) =delete;
    protected:
    noncopyable() =default;
    ~noncopyable() =default;
};