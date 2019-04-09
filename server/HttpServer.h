/*
** HttpServer.h
** 
**/
#pragma once
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
#include <unordered_map>
#include <map>
#include <memory>


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
	std::string getHeader();
	void handleGET();
	void handlePOST();
	void handleHEAD();

	void handleRead();
	void handleWrite();

	static std::string getType(const std::string &filetype);
	static void fileTypeInit();

	struct epoll_event ev, event[MAX_NFDS];
	int fd_;
	std::string inBuffer_;
	std::string outBuffer_;
	std::string fileName_;
	std::string outHeader_;
	std::map<std::string, std::string> header_;
	HttpVersion http_version_;
	HttpMethod method_;

	static const int MAX_NFDS;
	static const int buffer_size;
	static std::unordered_map<std::string, std::string> fileType;
	static pthread_once_t once_control;
}