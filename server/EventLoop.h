/* EventLoop.h
** ETZhangSX
*/
#pragma once
#include "base/Thread.h"
#include "Epoll.h"
#include "base/CurrentThread.h"
#include "Util.h"
#include "Channel.h"
#include <vector>
#include <functional>
#include <iostream>

using namespace std;

class EventLoop {
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void runInLoop(Functor&& func);
    void addToQueue(Functor&& func);
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    void assertInLoopThread() {
        assert(isInLoopThread());
    }

    void shutDown(shared_ptr<Channel> channel) {
        // shutDownWR(channel->getFd());
        shutdown(channel->getFd(), SHUT_WR);
    }

    void removeFromPoller(shared_ptr<Channel> channel) {
        poller_->epoll_del(channel);
    }

    void updatePoller(shared_ptr<Channel> channel) {
        poller_->epoll_mod(channel);
    }

    void addToPoller(shared_ptr<Channel> channel) {
        poller_->epoll_add(channel);
    }

    
private:
    bool looping_;
    int wakeupFd_;
    bool quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;
    mutable MutexLock mutex_;
    vector<Functor> pendingFunctors_;
    shared_ptr<Channel> pwakeupChannel_;
    shared_ptr<Epoll> poller_;

    const pid_t threadId_;

    void wakeup();
    void handleRead();
    void doPendingFunctors();
    void handleConn();
};