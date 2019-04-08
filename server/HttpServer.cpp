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
#include <sstream>
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

//解析URL
void HttpServer::parseURI() {
	string &uri = inBuffer_;
	// string对象的find(string &str, int pos)函数在调用的string对象
	//中查找pos开始后的匹配str的第一个位置
	size_t pos = uri.find('\r', 0);
	if (pos < 0) {
		return;
	}

	//截取请求行
	string request_line = uri.substr(0, pos);
	//去掉请求行
	if (uri.size() > pos + 1)
		uri = uri.substr(pos + 1);
	//创建请求行的string流
	stringstream input(request_line);

	string method;
	string path;
	string http_version;

	//获取请求行参数
	input >> method;
	input >> path;
	input >> http_version;

	switch (method) {
		case "GET":
			method_ = METHOD_GET;
			break;
		case "POST":
			method_ = METHOD_POST;
			break;
		case "HEAD":
			method_ = METHOD_HEAD;
			break;

		default:
			break;
	}

	//get filename
	if (path == "/" || path == "home") {
		fileName_ = "index.html";
	}
	else {
		// path = path.substr(1);
		path = "." + path;
		size_t posm = path.find('?');
		if (posm >= 0) {
			fileName_ = path.substr(0, posm);
		}
	}

	//get http version
	if (http_version.find("1.0") > 0) {
		http_version_ = HTTP_10;
	}
	else if (http_version.find("1.1") > 0) {
		http_version_ = HTTP_11;
	}
	else return;
}

//分析头部
void HttpServer::parseHeader() {
	string &headers = inBuffer_;
	string key_value;
	int pos = headers.find('\r');
	while (pos > 0) {
		key_value = headers.substr(0, pos);
		int k_pos = key_value.find(':');
		header_[key_value.substr(0, k_pos)] = key_value.substr(k_pos + 2);

		if (headers.length() > pos + 2) {
			//判断是否到达头部末尾
			if (headers[pos + 2] == '\r') {
				//判断是否存在data域
				if (headers.length() > pos + 3)
					headers = headers.substr(pos + 4);
				else headers = "";

				break;
			}
			headers = headers.substr(pos + 2);
		}
		else break;
		pos = headers.find('\r');
	}
}

//处理请求
void HttpServer::requestHandling() {
	if (method_ == METHOD_POST) {

	}
	else if (method_ == METHOD_GET) {

	}
	else if (method_ == METHOD_HEAD) {

	}
}

string HttpServer::getType() {
}

string HttpServer::getHeader(string content_type, int content_length) {
	string header = "HTTP/1.1 200 OK\r\n";
    
    header += "Server: A Simple Web Server\r\n"
    header += "Content-Type: " + content_type + "\r\n";
    header += "Content-Range: bytes\r\n";
    header += "Content-Length: " + to_string(content_length);
    header += "\r\n\r\n";

    return header;
}

//处理GET请求
void HttpServer::handleGET() {

}

//处理POST请求
void HttpServer::handlePOST() {
}

//处理HEAD请求
void HttpServer::handleHEAD() {
}

void HttpServer::handleRead() {
	
}

void HttpServer::handleWrite() {}

void HttpServer::connection() {}

//处理错误
void HttpServer::errorHandling() {}

