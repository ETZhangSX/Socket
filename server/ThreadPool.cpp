/*
** ThreadPool.cpp
** ETZhangSX
*/
#include "ThreadPool.h"

ThreadPool::ThreadPool(EventLoop* baseLoop, int numThreads):
    baseLoop_(baseLoop),
    started_(false),
    numThreads_(numThreads),
    next_(0) {
        if (numThreads_ <= 0) {
            cout << "Number of Threads <= 0\n";
            abort();
        }
}

void ThreadPool::start() {
    baseLoop_->assertInLoopThread();
    started_ = true;

    for (int i = 0; i < numThreads_; i++) {
        std::shared_ptr<EventLoopThread> thr(new EventLoopThread());
        threads_.push_back(thr);
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