/*
** Main.cpp
**
*/
#include "HttpServer.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

const int port = 80;
const int MAX_EVENTS = 256;
const int TIMEOUT = 500;
const int MAX_COM = 512;
const MAX_CON = 512;

struct epoll_event ev, event[MAX_EVENT];

void epollHandling(int epfd, int pos);
void handleError(const string &msg);

int main() {
	int listen_sock;
	int client_sock;

	int epfd;
	int nfds;

	epfd = epoll_create(MAX_EVENT);
	struct sockaddr_in listen_addr;
	struct sockaddr_in client_addr;

	socklen_t client_address_size;
	listen_sock = socket(PF_INET, SOCK_STREAM, 0);

	ev.data.fd = listen_sock;
	ev.events = EPOLLIN|EPOLLET;

	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev);

	if (listen_sock == -1) {
		handleError("socket error");
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(port);

	if (bind(listen_sock, (struct sockaddr*) &listen_addr, sizeof(listen_addr)) == -1) {
		handleError("bind error");
	}

	if (listen(listen_sock, 5) == -1) {
		handleError("listen error");
	}

	while (true) {
		nfds = epoll_wait(epfd, event, MAX_EVENT, TIMEOUT);

		for (int i = 0; i < nfds; i++) {
			if (event[i].data.fd == listen_sock) {
				client_address_size = sizeof(client_addr);
				client_sock = accept(listen_sock, (struct sockaddr*) &client_addr, &client_address_size);

				if (client_sock == -1) {
					handleError("accept error");
				}

				char *str = inet_ntoa(client_addr.sin_addr);
                cout << "accept from " << str << endl;

                //注册事件描述符
                ev.data.fd = client_sock;
                //注册事件的类型
                ev.events = EPOLLIN|EPOLLET;
                //注册事件
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock, &ev);
			}
			else {
				epollHandling(epfd, i);
			}
		}
	}
	close(server_sock);
    close(epfd);
    return 0;
}

//处理epoll事件
void epollHandling(int epfd, int pos) {
    int client_sock = event[pos].data.fd;
    char buffer[buffer_size];
    char method[method_size];
    char filename[filename_size];

    string t_method;
    string t_filename;
    string t_httpversion;

    if (event[pos].events & EPOLLIN) {
            
        cout << "EPOLLIN" << endl;

        if (client_sock < 0) {
            return;
        }
        
        

        //判断是否是HTTP请求
        // if (!strstr(buffer, "HTTP/")) {
        //     sendError(&client_sock);
        //     epoll_ctl(epfd, EPOLL_CTL_DEL, client_sock, NULL);
        //     close(client_sock);
        //     return;
        // }
    
        // strcpy(method, strtok(buffer, " /"));
        // strcpy(filename, strtok(NULL, " /"));
    
        // if (0 == strcmp(filename, "HTTP") || 0 == strcmp(filename, "home"))
        //     strcpy(filename, "index.html");

        // if (0 != strcmp(method, "GET")) {
        //     sendError(&client_sock);
        //     epoll_ctl(epfd, EPOLL_CTL_DEL, client_sock, NULL);
        //     close(client_sock);
        //     return;
        // }

        //修改注册事件
        ev.data.fd = client_sock;
        ev.events = EPOLLOUT|EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_MOD, client_sock, &ev);
    }
    else if (event[pos].events & EPOLLOUT) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, client_sock, NULL);
    }
}

void handleError(const string &msg) {
    cout << msg;
    exit(1);
}