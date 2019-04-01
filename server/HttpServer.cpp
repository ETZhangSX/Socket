/*
** HttpServer.cpp
** 
**/
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string>
#include "HttpServer.h"

using namespace std;

void HttpServer::start() {
	int listen_sock;
		//epoll句柄
		int epfd;
		//事件发生数
		int nfds;

		epfd = epoll_create(MAX_NFDS);
		struct sockaddr_in server_addr;
		struct sockaddr_in client_addr;

		socklen_t client_addr_size;

		//创建套接字
		listen_sock = socket(PE_INET, SOCK_STREAM, 0);
		ev.data.fd = listen_sock;
		ev.events = EPOLLIN|EPOLLET;
		epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock, &ev);

		if (listen_sock == -1) {

		}

		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(port);
}

void HttpServer::parseRequest() {
}

void HttpServer::connection() {}

void HttpServer::errorHandling() {}

