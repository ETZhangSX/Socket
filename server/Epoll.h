/*
** Epoll.h
** ETZhangSX
*/
#pragma once

#include "Channel.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>

class Epoll {
public:
    Epoll();
    ~Epoll();
    void epoll_add(SP_Channel request);
    void epoll_mod(SP_Channel request);
    void epoll_del(SP_Channel request);
    std::vector<std::shared_ptr<Channel>> poll();
    
private:
    static const int MAXFDS = 10000;
    int epollFd_;
    std::vector<epoll_event> events_;
    std::shared_ptr<Channel> fd_channel_[MAXFDS];
    std::shared_ptr<HttpServer> fd_http_[MAXFDS];
}