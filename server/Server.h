/*
** Server.h
** ETZhangSX
*/
#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "ThreadPool.h"
#include <memory>

class Server {
public:
    Server(EventLoop* loop, int threadNumber, int port);
    ~Server() {}
    EventLoop* getLoop() const { return loop_; }
    void start();
    void newConn();
    void curConn();

private:
    EventLoop* loop_;
    int threadNumber_;
    std::unique_ptr<ThreadPool> threadPool_;
    bool started_;
    std::shared_ptr<Channel> acceptChannel_;
    int port_;
    int listenFd_;
    static const int MAXFD = 1000;
}