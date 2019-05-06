/*
** Epoll.cpp
** ETZhangSX
*/
#pragma once
#include "Epoll.h"
#include "Util.h"
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <queue>
#include <deque>
#include <assert.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

typedef shared_ptr<Channel> SP_Channel;

Epoll::Epoll():
    epollFd_(epoll_create1(EPOLL_CLOEXEC)),
    events_(EVENTSNUM) {
        assert(epollFd_ > 0);
}

Epoll::~Epoll() {}

void Epoll::epoll_add(SP_Channel request) {
    int fd = request->getFd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();

    request->EqualAndUpdateLastEvents();

    fd_channel_[fd] = request;

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
        perror("epoll_add error");
        fd_channel_[fd].reset();
    }
}

void Epoll::epoll_mod(SP_Channel request) {
    int fd = request->getFd();
    if (!request->EqualAndUpdateLastEvents()) {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->getEvents();
        if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
            perror("epoll_mod error");
            fd_channel_[fd].reset();
        }
    }
}

void Epoll::epoll_del(SP_Channel request) {
    int fd = request->getFd();
    // struct epoll_event event;
    // event.data.fd = fd;
    // event.events = request->getLastEvents();

    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, NULL) < 0) {
        perror("epoll_del error");
    }

    fd_channel_[fd].reset();
    fd_http_[fd].reset();
}

vector<SP_Channel> Epoll::poll() {
    while (true) {
        int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
        if (event_count < 0) {
            perror("epoll wait error");
        }
        vector<SP_Channel> req_data;
        for (int i = 0; i < event_count; i++) {
            int fd = events_[i].data.fd;
            SP_Channel cur_req = fd_channel_[fd];

            if (cur_req) {
                cur_req->setRevents(events_[i].events);
                cur_req->setEvents(0);

                req_data.push_back(cur_req);
            }
            else {
                cout << "SP current request is invalid" << endl;
            }
        }
        if (req_data.size() > 0) {
            return req_data;
        }
    }
}
