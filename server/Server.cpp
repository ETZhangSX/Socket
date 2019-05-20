//
//  server.cpp
//  test
//
//  Created by 张顺鑫 on 2019/3/9.
//  Copyright © 2019 张顺鑫. All rights reserved.
//

#include "Server.h"
#include "Util.h"
#include "HttpServer.h"
#include <functional>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;


SSL_CTX *ctx = NULL;

void SSL_init() {
    //const SSL_METHOD *method = NULL;
    // SSL *ssl = NULL;
    //SSL初始化
    SSL_library_init();

    const SSL_METHOD *method = SSLv23_server_method();
    ctx = SSL_CTX_new(method);

    SSL_CTX_load_verify_locations(ctx, CHAIN, NULL);
    SSL_CTX_use_certificate_file(ctx, CERTSERVER, SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, KEYSERVER, SSL_FILETYPE_PEM);
    
    SSL_CTX_check_private_key(ctx);
}

Server::Server(EventLoop* loop, int threadNumber, int port):
    loop_(loop),
    threadNumber_(threadNumber),
    threadPool_(new ThreadPool(loop_, threadNumber_)),
    started_(false),
    acceptChannel_(new Channel(loop_)),
    port_(port),
    listenFd_(socket_bind_listen(port_)) {
    SSL_init();
    acceptChannel_->setFd(listenFd_);
    handle_sigpipe();
    if (setSocketNonBlocking(listenFd_) < 0) {
        perror("set socket non block failed");
        abort();
        SSL_init();
    }
}

void Server::start() {
    cout << "Starting Thread Pool\n";
    threadPool_->start();
    cout << "Start Successfully\nSetup accept channel\n";
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    acceptChannel_->setReadHandler(bind(&Server::newConn, this));
    acceptChannel_->setConnHandler(bind(&Server::curConn, this));
    cout << "Adding channel to poller\n";
    loop_->addToPoller(acceptChannel_);
    cout << "Add successfully\n";
    started_ = true;
}

Server::~Server() {
    SSL_CTX_free(ctx);
}

void Server::newConn() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;

    while((accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0) {
        EventLoop* loop = threadPool_->getNextLoop();
        cout << "new connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << '\n';
        cout << "Epoll Fd: " << loop->getPollerFd() << " Accept_fd: " << accept_fd << '\n';
        if (accept_fd > MAXFD) {
            close(accept_fd);
            continue;
        }

        if (setSocketNonBlocking(accept_fd) < 0) {
            cout << "set Socket non block failed\n";
            return;
        }

        setTCPNoDelay(accept_fd);

        shared_ptr<HttpServer> request_info(new HttpServer(loop, accept_fd, ctx));
        request_info->getChannel()->setHolder(request_info);
        loop->addToQueue(std::bind(&HttpServer::newEvent, request_info));
    }
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}