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
#include <sys/socket.h>
#include <iostream>
#include <assert.h>

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

    void shutDown(SP_Channel channel) {
        // shutDownWR(channel->getFd());
        shutdown(channel->getFd(), SHUT_WR);
    }

    void removeFromPoller(SP_Channel channel) {
        poller_->epoll_del(channel);
    }

    void updatePoller(SP_Channel channel) {
        poller_->epoll_mod(channel);
    }

    void addToPoller(SP_Channel channel) {
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