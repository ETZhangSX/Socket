/*
** EventLoopThread.cpp
** ETZhangSX
*/
#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread():
    loop_(NULL),
    exiting_(false),
    thread_(bind(&EventLoopThread::threadFunction, this), "EventLoopThread"),
    mutex_(),
    cond_(mutex_) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != NULL) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();
    mutex_.lock();
    while (loop_ == NULL) {
        cond_.wait();
    }
    mutex_.unlock();
    
    return loop_;
}

void EventLoopThread::threadFunction() {
    EventLoop loop;

    mutex_.lock();
    loop_ = &loop;
    cond_.notify();
    mutex_.unlock();

    loop.loop();

    loop_ = NULL;
}