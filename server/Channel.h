/*
** Channel.h
** ETZhangSX
*/
#pragma once
#include "base/epoll.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <iostream>

class EventLoop;
class HttpServer;

class Channel {
private:
    typedef std::function<void()> CallBack;

public:
    Channel(EventLoop* loop);
    Channel(EventLoop* loop, int fd);
    ~Channel();
    int getFd();        //获取连接描述符
    void setFd(int fd); //设置连接描述符

    //执行绑定的对应函数
    void handleRead();
    void handleWrite();
    void handleConn();
    void handleError();

    //设置持有的HttpServer对象
    void setHolder(std::shared_ptr<HttpServer> holder) {
        holder_ = holder;
    }

    //获取HttpServer对象
    std::shared_ptr<HttpServer> getHolder() {
        std::shared_ptr<HttpServer> ret(holder_.lock());
        return ret;
    }

    //设置绑定对象的函数
    void setReadHandler(CallBack &&readHandler) {
        readHandler_ = readHandler;
    }

    void setWriteHandler(CallBack &&writeHandler) {
        writeHandler_ = writeHandler;
    }

    void setErrorHandler(CallBack &&errorHandler) {
        errorHandler_ = errorHandler;
    }

    void setConnHandler(CallBack &&connHandler) {
        connHandler_ = connHandler;
    }

    //处理事件
    void handleEvents() {
        std::cout << "\033[32;1mChannel::\033[0mhandleEvents() " << getFd() << '\n';
        events_ = 0;
        if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
            events_ = 0;
            return;
        }

        if (revents_ & EPOLLERR) {
            if (errorHandler_) errorHandler_();
            events_ = 0;
            return;
        }

        if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
            handleRead();
        }

        if (revents_ & EPOLLOUT) {
            handleWrite();
        }

        handleConn();
    }

    void setRevents(__uint32_t ev) {
        revents_ = ev;
    }

    void setEvents(__uint32_t ev) {
        events_ = ev;
    }

    __uint32_t& getEvents() {
        return events_;
    }

    bool EqualAndUpdateLastEvents() {
        bool ret = (lastEvents_ == events_);
        lastEvents_ = events_;
        return ret;
    }

    __uint32_t getLastEvents() {
        return lastEvents_;
    }

    typedef std::shared_ptr<Channel> SP_Channel;

private:
    EventLoop *loop_;
    int fd_;
    __uint32_t events_;
    __uint32_t revents_;
    __uint32_t lastEvents_;

    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

    std::weak_ptr<HttpServer> holder_;
};

typedef std::shared_ptr<Channel> SP_Channel;
