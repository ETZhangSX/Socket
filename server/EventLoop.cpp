/* EventLoop.cpp
** ETZhangSX
*/
#include "EventLoop.h"
#include "Util.h"
#include "base/epoll.h"
#include <sys/eventfd.h>
#include <iostream>

using namespace std;

__thread EventLoop* t_loopInThisThread = 0;

int creatEventfd() {
    int evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0) {
        abort();
    }
    return evfd;
}

EventLoop::EventLoop() :
    looping_(false),
    wakeupFd_(creatEventfd()),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    pwakeupChannel_(new Channel(this, wakeupFd_)),
    poller_(new Epoll()),
    threadId_(CurrentThread::tid()) {
        if (t_loopInThisThread) {
            cout << "An EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_ << endl;
        }
        else {
            t_loopInThisThread = this;
        }

        pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
        pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
        pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
        poller_->epoll_add(pwakeupChannel_);
}

void EventLoop::handleConn() {
    updatePoller(pwakeupChannel_);
}

EventLoop::~EventLoop() {
    close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd_, (char *)(&one), sizeof one);
    if (n != sizeof one) {
        cout << "wakeup writes " << n << " bytes\n";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = readn(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        cout << "handleRead reads " << n << " bytes\n";
    }
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& func) {
    if (isInLoopThread()) {
        func();
    }
    else {
        addToQueue(move(func));
    }
}

void EventLoop::addToQueue(Functor&& func) {
    mutex_.lock();
    pendingFunctors_.emplace_back(move(func));
    mutex_.unlock();

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::loop() {
    cout << "\033[32;1mEventLoop::\033[0mloop() " << this->getPollerFd() << '\n';
    assert(!looping_);
    assert(isInLoopThread());
    looping_ = true;
    quit_ = false;

    vector<SP_Channel> ret;

    while (!quit_) {
        ret.clear();
        ret = poller_->poll();
        eventHandling_ = true;
        for (auto &item: ret) {
            cout << "\033[32;1mEventLoop::\033[0mEvent Fd: " << item->getFd() << '\n';
            item->handleEvents();
        }
        eventHandling_ = false;
        doPendingFunctors();
    }
    looping_ = false;
}

void EventLoop::doPendingFunctors() {
    vector<Functor> functors;
    callingPendingFunctors_ = true;
    
    mutex_.lock();
    functors.swap(pendingFunctors_);
    mutex_.unlock();

    cout << "EventLoop::doPendingFunctors : " << functors.size() << '\n';

    for (size_t i = 0; i < functors.size(); i++) {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}
