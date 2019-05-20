/*
** HttpServer.h
** ETZhangSX
**/
#pragma once
#include "base/epoll.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <openssl/ssl.h>

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
	HttpServer(EventLoop* loop, int fd, SSL_CTX *ctx);
	~HttpServer();
	std::shared_ptr<Channel> getChannel() { return channel_; }
	EventLoop* getLoop() { return loop_; }

	void reset();
	
	URIState parseURI();
	void parseHeader();
	AnalysisState requestHandling();
	void connection();
	void errorHandling();
	void newEvent();
	
private:
	std::string getHeader(std::string content_type, int content_length);
	AnalysisState handleGET();
	AnalysisState handlePOST();
	AnalysisState handleHEAD();

	void handleRead();
	void handleWrite();
	void handleError(int fd, int err_num, std::string msg);
	void handleClose();

	static std::string getType(const std::string &filetype);
	static void fileTypeInit();

	EventLoop* loop_;
	std::shared_ptr<Channel> channel_;
	int fd_;
	SSL* ssl_;
	FILE* fp_;
	std::string inBuffer_;
	std::string outBuffer_;
	std::string fileName_;
	std::string outHeader_;
	std::map<std::string, std::string> header_;
	HttpMethod method_;
	HttpVersion http_version_;

	static const int buffer_size;
	static std::unordered_map<std::string, std::string> fileType;
	static pthread_once_t once_control;
};