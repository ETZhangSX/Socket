/*
** EventLoopThread.cpp
** ETZhangSX
*/
#include "EventLoopThread.h"
#include <functional>
#include <iostream>

using namespace std;

EventLoopThread::EventLoopThread():
    loop_(NULL),
    exiting_(false),
    thread_(bind(&EventLoopThread::threadFunction, this), "EventLoopThread"),
    mutex_(),
    cond_(mutex_) {
        cout << "EventLoopThread Constructor\n";
    }

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != NULL) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    cout << "In EventLoopThread::startLoop\n";
    assert(!thread_.started());
    cout << "Thread starting\n";
    thread_.start();
    mutex_.lock();
    while (loop_ == NULL) {
        cond_.wait();
    }
    mutex_.unlock();
    
    cout << "Thread starting Finished\n";
    return loop_;
}

void EventLoopThread::threadFunction() {
    cout << "threadFunction\n";
    EventLoop loop;

    mutex_.lock();
    loop_ = &loop;
    cond_.notify();
    mutex_.unlock();

    cout << "Starting loop\n";
    loop.loop();

    loop_ = NULL;
    cout << "threadFunction finished\n";
}