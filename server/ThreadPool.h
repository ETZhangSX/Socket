/*
** ThreadPool.h
** ETZhangSX
*/
#pragma once
#include "EventLoopThread.h"
#include <memory>
#include <vector>

class ThreadPool
{
private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;

public:
    ThreadPool(EventLoop* baseLoop, int numThreads);
    ~ThreadPool() {
        cout << "Thread Pool destruct\n";
    }

    void start();
    EventLoop* getNextLoop();
};