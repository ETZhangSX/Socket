/*
** HttpServer.cpp
** ETZhangSX
**/
#include "HttpServer.h"
#include "Channel.h"
#include "Util.h"
#include "EventLoop.h"
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <sstream>

using namespace std;

const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET;// | EPOLLONESHOT;
const int HttpServer::buffer_size = 1<<20;

// 只初始化一次
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

HttpServer::HttpServer(EventLoop* loop, int fd): 
	loop_(loop),
	channel_(new Channel(loop, fd)),
	fd_(fd),
	ssl_(SSL_new(ctx)),
	method_(METHOD_GET),
	http_version_(HTTP_11) {
	if (SSL_set_fd(ssl_, fd) != 1){
		cout << "SSL set fd error\n";
	}
	if (SSL_accept(ssl_) != 1) {
		cout << "SSL accept error\n";
	}
	channel_->setReadHandler(bind(&HttpServer::handleRead, this));
	channel_->setWriteHandler(bind(&HttpServer::handleWrite, this));
	channel_->setConnHandler(bind(&HttpServer::connection, this));
}

HttpServer::~HttpServer() {
	cout << "\033[32;1m~HttpServer\033[0m\n";
	close(fd_);
	SSL_free(ssl_);
}

void HttpServer::reset() {
	fileName_.clear();
	header_.clear();
}

//解析URL
URIState HttpServer::parseURI() {
	string &uri = inBuffer_;
	// string对象的find(string &str, int pos)函数在调用的string对象
	//中查找pos开始后的匹配str的第一个位置
	size_t pos = uri.find('\r', 0);
	if (pos < 0) {
		return PARSE_URI_ERROR;
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

	if ("GET" == method) {
		method_ = METHOD_GET;
	}
	else if ("POST" == method) {
		method_ = METHOD_POST;
	}
	else if ("HEAD" == method) {
		method_ = METHOD_HEAD;
	}
	else {
		return PARSE_URI_ERROR;
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
	else return PARSE_URI_ERROR;

	return PARSE_URI_SUCCESS;
}

//分析头部
void HttpServer::parseHeader() {
	string &headers = inBuffer_;
	string key_value;
	int pos = headers.find('\r');
	int k_pos = 0;

	//获取表头所有信息
	while (pos > 0) {
		key_value = headers.substr(0, pos);
		k_pos = key_value.find(':');
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
AnalysisState HttpServer::requestHandling() {
	if (method_ == METHOD_POST) {
		return handlePOST();
	}
	else if (method_ == METHOD_GET) {
		return handleGET();
	}
	else if (method_ == METHOD_HEAD) {
		return handleHEAD();
	}
	else return ANALYSIS_ERROR;
}

//生成响应头
string HttpServer::getHeader(string content_type, int content_length) {
	string header = "HTTP/1.1 200 OK\r\n";
    
    header += "Server: A Simple Web Server\r\n";
    header += "Content-Type: " + content_type + "\r\n";
    header += "Content-Range: bytes\r\n";
    header += "Content-Length: " + to_string(content_length);
    header += "\r\n\r\n";

    return header;
}

//处理GET请求
AnalysisState HttpServer::handleGET() {
	FILE *fp;
	string filetype;
	string readOpenMode;
	string header;

	int pos = fileName_.find('.', 1);
	if (pos < 0) {
		filetype = "default";
	}
	else filetype = fileName_.substr(pos);

	if (filetype == ".html" || filetype == ".htm") {
		readOpenMode = "r";
	}
	else {
		readOpenMode = "rb";
	}

	fp = fopen(fileName_.c_str(), readOpenMode.c_str());

	if (NULL == fp) {
		//TODO:文件打开错误
		handleError(fd_, 404, "Not Found");
		return ANALYSIS_ERROR;
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
	fp_ = fp;
	handleWrite();
	cout << "Finish reading\n";
	fp_ = NULL;
	fclose(fp);

	return ANALYSIS_SUCCESS;
}

//处理POST请求
AnalysisState HttpServer::handlePOST() {
	return ANALYSIS_SUCCESS;
}

//处理HEAD请求
AnalysisState HttpServer::handleHEAD() {
	return ANALYSIS_SUCCESS;
}

void HttpServer::handleRead() {
	bool isZero = false;
	int read_num = ssl_read(ssl_, inBuffer_, isZero);

	cout << "Request line: \n" << inBuffer_;

	if (read_num < 0) {
		perror("Readn() error");
		// 处理错误
		handleError(fd_, 404, "Bad Request");
		return;
	}
	else if (isZero) {
		if (read_num == 0) {
			perror("No data");
			return;
		}
	}

	URIState &&flag = this->parseURI();
	if (flag == PARSE_URI_ERROR) {
		perror("parseURI error");
		inBuffer_.clear();
		handleError(fd_, 404, "Bad Request");
		return;
	}
	this->parseHeader();
	AnalysisState &&req_flag = this->requestHandling();
	if (req_flag == ANALYSIS_SUCCESS) {
		this->reset();
	}
}

void HttpServer::handleWrite() {
	__uint32_t &events_ = channel_->getEvents();

	if (ssl_write(ssl_, outBuffer_) < 0) {
		perror("writen error");
		events_ = 0;
	}
	else if (fp_ != NULL) {
		ssl_writeFile(fp_, ssl_);
	}
	if (outBuffer_.size() > 0) {
		events_ |= EPOLLOUT;
	}
}

void HttpServer::connection() {
	cout << "\033[32;1mHttpServer::\033[0mconnection() \n";
	__uint32_t &events_ = channel_->getEvents();
	if (events_ != 0) {
		if ((events_ & EPOLLIN) && (events_ & EPOLLOUT)) {
			events_ = __uint32_t(0);
			events_ |= EPOLLOUT;
		}
		events_ |= EPOLLET;
		loop_->updatePoller(channel_);
	}
	else {
		events_ |= (EPOLLIN | EPOLLET);
		loop_->updatePoller(channel_);
	}
	loop_->runInLoop(bind(&HttpServer::handleClose, shared_from_this()));
}

//处理错误，回复错误信息
void HttpServer::handleError(int fd, int err_num, string msg) {
	msg = " " + msg;
    char send_buff[4096];
    string body_buff, header_buff;
    body_buff += "<html><title>哎~出错了</title>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += to_string(err_num) + msg;
    body_buff += "<hr><em> ET Web Server</em>\n</body></html>";

    header_buff += "HTTP/1.1 " + to_string(err_num) + msg + "\r\n";
    header_buff += "Content-Type: text/html\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: ET Web Server\r\n";;
    header_buff += "\r\n";
    // 错误处理不考虑writen不完的情况
    sprintf(send_buff, "%s", header_buff.c_str());
    ssl_write(ssl_, send_buff, strlen(send_buff));
    sprintf(send_buff, "%s", body_buff.c_str());
    ssl_write(ssl_, send_buff, strlen(send_buff));
}

void HttpServer::handleClose() {
	cout << "\033[32;1mHttpServer::\033[0mhandleClose() \n";
	shared_ptr<HttpServer> guard(shared_from_this());
	loop_->removeFromPoller(channel_);
}

void HttpServer::newEvent() {
	cout << "\033[32;1mHttpServer::\033[0mnewEvent() \n";
	channel_->setEvents(DEFAULT_EVENT);
	loop_->addToPoller(channel_);
	cout << "\033[32;1mHttpServer::\033[0mnewEvent() \033[31;1mFinished\033[0m \n";
}