/*
** Epoll.h
** ETZhangSX
*/
#pragma once

#include "Channel.h"
#include "base/epoll.h"
#include <vector>
#include <unordered_map>
#include <memory>

// class Channel;

class Epoll {
public:
    Epoll();
    ~Epoll();
    void epoll_add(SP_Channel request);
    void epoll_mod(SP_Channel request);
    void epoll_del(SP_Channel request);
    std::vector<SP_Channel> poll();
    int getEpollFd() { return epollFd_; }
private:
    static const int MAXFDS = 10000;
    int epollFd_;
    std::vector<epoll_event> events_;
    SP_Channel fd_channel_[MAXFDS];
    std::shared_ptr<HttpServer> fd_http_[MAXFDS];
};