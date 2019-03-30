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

using namespace std;

enum HttpMethod
{
    METHOD_POST = 1,
    METHOD_GET,
    METHOD_HEAD
};

enum HttpVersion
{
    HTTP_10 = 1,
    HTTP_11
};

class HttpServer {
public:
	HttpServer(){}
	~HttpServer(){}
	void start(){
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
	void parseRequest(){}
	void connection(){}
	void errorHandling(){}
private:
	static const int MAX_NFDS = 500;
	struct epoll_event ev, event[MAX_NFDS];
	int fd_;
	string inBuffer_;
	string outBuffer_;
	string fileName_;
	
}