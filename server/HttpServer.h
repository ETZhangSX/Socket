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
#include <map>


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
	
	void parseURI();
	void parseHeader();
	void requestHandling();
	void connection();
	void errorHandling();
	
private:
	string getHeader();
	void handleGET();
	void handlePOST();
	void handleHEAD();

	void handleRead();
	void handleWrite();

	static const int MAX_NFDS = 500;
	struct epoll_event ev, event[MAX_NFDS];
	int fd_;
	std::string inBuffer_;
	std::string outBuffer_;
	std::string fileName_;
	std::string outHeader_;
	std::map<string, string> header_;
	HttpVersion http_version_;
	HttpMethod method_;
}