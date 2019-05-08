/*
** ThreadPool.cpp
** ETZhangSX
*/
#include "ThreadPool.h"
#include <iostream>

using namespace std;


ThreadPool::ThreadPool(EventLoop* baseLoop, int numThreads):
    baseLoop_(baseLoop),
    started_(false),
    numThreads_(numThreads),
    next_(0) {
        if (numThreads_ <= 0) {
            cout << "Number of Threads <= 0\n";
            abort();
        }
        cout << "Thread Pool Constructor\n";
}

void ThreadPool::start() {
    cout << "assertInLoopThread\n";
    baseLoop_->assertInLoopThread();
    started_ = true;

    cout << "Creating Threads\n";
    for (int i = 0; i < numThreads_; i++) {
        cout << "Create thread " << i << '\n';
        std::shared_ptr<EventLoopThread> thr(new EventLoopThread());
        cout << "Pushing back thread\n";
        threads_.push_back(thr);
        cout << "Pushing back loop\n";
        loops_.push_back(thr->startLoop());
    }
}

EventLoop* ThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        next_ = (next_ + 1) % numThreads_;
    }

    return loop;
}