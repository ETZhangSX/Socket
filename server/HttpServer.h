/*
** HttpServer.h
** ETZhangSX
**/
#pragma once
#include "Channel.h"
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

class EventLoop;
class Channel;

enum HttpMethod
{
    METHOD_POST = 1,
    METHOD_GET,
    METHOD_HEAD
};

enum URIState
{
    PARSE_URI_AGAIN = 1,
    PARSE_URI_ERROR,
    PARSE_URI_SUCCESS,
};

enum HeaderState
{
    PARSE_HEADER_SUCCESS = 1,
    PARSE_HEADER_AGAIN,
    PARSE_HEADER_ERROR
};

enum AnalysisState
{
    ANALYSIS_SUCCESS = 1,
    ANALYSIS_ERROR
};

enum HttpVersion
{
    HTTP_10 = 1,
    HTTP_11
};

class HttpServer : public std::enable_shared_from_this<HttpServer> {
public:
	HttpServer(EventLoop* loop, int fd){}
	~HttpServer(){ close(fd_); }
	std::shared_ptr<Channel> getChannel() { return channel_; }
	EventLoop* getLoop() { return loop_; }

	// void start();
	void reset();
	
	URIState parseURI();
	void parseHeader();
	AnalysisState requestHandling();
	void connection();
	void errorHandling();
	void newEvent();
	
private:
	std::string getHeader();
	AnalysisState handleGET();
	AnalysisState handlePOST();
	AnalysisState handleHEAD();

	void handleRead();
	void handleWrite(FILE *fp);
	void handleError(int fd, int err_num, std::string msg);
	void handleClose();

	static std::string getType(const std::string &filetype);
	static void fileTypeInit();

	struct epoll_event ev, event[MAX_NFDS];
	EventLoop* loop_;
	std::shared_ptr<Channel> channel_;
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