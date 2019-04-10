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

int HttpServer::MAX_NFDS = 500;
int HttpServer::buffer_size = 1<<20;

pthread_once_t HttpServer::once_control = PTHREAD_ONCE_INIT;
unordered_map<string, string> HttpServer::fileType;

void HttpServer::fileTypeInit() {
	fileType[".html"] = "text/html";
    fileType[".avi"] = "video/x-msvideo";
    fileType[".bmp"] = "image/bmp";
    fileType[".c"] = "text/plain";
    fileType[".doc"] = "application/msword";
    fileType[".gif"] = "image/gif";
    fileType[".gz"] = "application/x-gzip";
    fileType[".htm"] = "text/html";
    fileType[".ico"] = "image/x-icon";
    fileType[".jpg"] = "image/jpeg";
    fileType[".png"] = "image/png";
    fileType[".txt"] = "text/plain";
    fileType[".mp3"] = "audio/mp3";
    fileType["default"] = "text/html";
}

string HttpServer::getType(const string &filetype) {
	pthread_once(&once_control, HttpServer::fileTypeInit);
	if (fileType.find(filetype) == fileType.end()) {
		return fileType["default"];
	}
	else {
		return fileType[filetype];
	}
}

HttpServer::HttpServer(int fd): fd_(fd) {

}

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
		handlePOST();
	}
	else if (method_ == METHOD_GET) {
		handleGET();
	}
	else if (method_ == METHOD_HEAD) {
		handleHEAD();
	}
}

//生成响应头
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
	FILE *fp;
	char buffer[buffer_size];
	string filetype;
	string readOpenMode;
	string header;

	int pos = fileName_.find('.', 1);
	if (pos < 0) {
		//TODO:错误处理
		return;
	}

	filetype = fileName_.substr(pos);

	if (filetype == ".html" || filetype == ".htm") {
		readOpenMode = "r";
	}
	else {
		readOpenMode = "rb";
	}

	fp = fopen(fileName_, readOpenMode);

	if (NULL == fp) {
		//TODO:文件打开错误
		fclose(fp);
		return;
	}

	fseek(fp, 0L, SEEK_END);
	int filelen = ftell(fp);

	outBuffer_ = getHeader(getType(filetype), filelen);

	cout << "Reading file: " << fileName_ << endl;

	fseek(fp, 0L, SEEK_SET);

	// while (!feof(fp)) {
	// 	fread(buffer, sizeof(char), sizeof(buffer), fp);
	// 	outBuffer_ += string(buffer, buffer + sizeof(buffer));
	// }

	// 上述发送文件过程对相对大的文件内存可能不够，故采用边读边发
	// 发送响应头和文件
	handleWrite(fp);

	cout << "Finish reading\n";
	fclose(fp);
}

//处理POST请求
void HttpServer::handlePOST() {
}

//处理HEAD请求
void HttpServer::handleHEAD() {
}

void HttpServer::handleRead() {
	bool isZero = false;
	int read_num = readn(fd_, inBuffer_, isZero);

	cout << "Request line: \n" << inBuffer_;

	if (read_num < 0) {
		perror("Readn() error");
		// 处理错误
		return;
	}
	else if (isZero) {
		if (read_num == 0) {
			perror("No data");
			return;
		}
	}

}

void HttpServer::handleWrite(FILE *fp) {
	writen(fd_, outBuffer_);
	writeFile(fp, &fd_);
}

void HttpServer::connection() {}

//处理错误
void HttpServer::errorHandling() {}

