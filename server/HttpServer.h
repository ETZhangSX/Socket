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
	void start();
	void parseRequest();
	void connection();
	void errorHandling();
	
private:
	static const int MAX_NFDS = 500;
	struct epoll_event ev, event[MAX_NFDS];
	int fd_;
	string inBuffer_;
	string outBuffer_;
	string fileName_;
	
}