//
//  server.cpp
//  test
//
//  Created by 张顺鑫 on 2019/3/9.
//  Copyright © 2019 张顺鑫. All rights reserved.
//
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h> //epoll头文件
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

using namespace std;

const int port = 80;
const int buffer_size = 1<<20;
const int method_size = 1<<10;
const int filename_size = 1<<10;
const int common_buffer_size = 1<<10;
const int MAX_EVENTS = 256;
const int TIMEOUT = 500;
const int MAX_CON = 512;

struct client_data
{
    char method[method_size];
    char filename[filename_size];
};
//声明epoll_event结构体的变量
struct epoll_event ev, event[MAX_EVENTS];
struct client_data cln_data[MAX_CON];

void setnonblocking(int sock);
void handleError(const string &msg);
void epollHandling(int epfd, int pos);
void requestHandling(int *sock);
void sendError(int *sock);
void sendData(int *sock, char *filename);
void sendHTML(int *sock, char *filename);
void sendJPG(int *sock, char *filename);

int main() {
    //声明套接字
    int server_sock;
    int client_sock;
    //声明epoll句柄
    int epfd;
    //声明事件发生数
    int nfds;

    //生成epoll句柄
    epfd = epoll_create(MAX_EVENTS);
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    
    socklen_t client_address_size;
    
    //创建套接字
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    // setnonblocking(server_sock);
    //设置相关描述符
    ev.data.fd = server_sock;
    //设置事件类型为 可读 边缘触发
    ev.events = EPOLLIN|EPOLLET;
    //注册epoll事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock, &ev);

    if (server_sock == -1) {
        handleError("socket error");
    }
    
    //初始化并设置套接字地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    
    //绑定
    if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
        handleError("bind error");
    }
    
    //监听
    if (listen(server_sock, 5) == -1) {
        handleError("listen error");
    }
    
    //等待消息传入
    while (true) {
        //等待epoll事件发生
        nfds = epoll_wait(epfd, event, MAX_EVENTS, TIMEOUT);

        //处理发生事件
        for (int i = 0; i < nfds; i++) {
            
            if (event[i].data.fd == server_sock) {
                
                client_address_size = sizeof(client_addr);
                client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_address_size);
                
                if (client_sock == -1) {
                    handleError("accept error");
                }
                // setnonblocking(client_sock);

                char *str = inet_ntoa(client_addr.sin_addr);
                cout << "accept from " << str << endl;

                ev.data.fd = client_sock;

                ev.events = EPOLLIN|EPOLLET;

                //注册事件
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock, &ev);
            }
            else {
                epollHandling(epfd, i);
            }
        }
        // requestHandling(&client_sock);
    }
    
    close(server_sock);
    close(epfd);
    return 0;
}

void setnonblocking(int sock) {
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts)");
        exit(1);
    }
}

//处理epoll事件
void epollHandling(int epfd, int pos) {
    int client_sock = event[pos].data.fd;
    char buffer[buffer_size];
    char method[method_size];
    char filename[filename_size];

    if (event[pos].events & EPOLLIN) {
            
        cout << "EPOLLIN" << endl;

        if (client_sock < 0) {
            return;
        }
        
        //读取数据到buffer
        read(client_sock, buffer, sizeof(buffer) - 1);

        //判断是否是HTTP请求
        if (!strstr(buffer, "HTTP/")) {
            sendError(&client_sock);
            epoll_ctl(epfd, EPOLL_CTL_DEL, client_sock, NULL);
            close(client_sock);
            return;
        }
    
        strcpy(method, strtok(buffer, " /"));
        strcpy(filename, strtok(NULL, " /"));
    
        if (0 == strcmp(filename, "HTTP") || 0 == strcmp(filename, "home"))
            strcpy(filename, "index.html");

        if (0 != strcmp(method, "GET")) {
            sendError(&client_sock);
            epoll_ctl(epfd, EPOLL_CTL_DEL, client_sock, NULL);
            close(client_sock);
            return;
        }

        //修改注册事件
        ev.data.fd = client_sock;
        ev.events = EPOLLOUT|EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_MOD, client_sock, &ev);

        //将读取信息保存
        strcpy(cln_data[client_sock].method, method);
        strcpy(cln_data[client_sock].filename, filename);
    }
    else if (event[pos].events & EPOLLOUT) {
        sendData(&client_sock, cln_data[client_sock].filename);
        epoll_ctl(epfd, EPOLL_CTL_DEL, client_sock, NULL);
    }
}

//用于原非epoll的简单socket实现版本
//由epoll替代
//处理请求
void requestHandling(int *sock) {
    int client_sock = *sock;
    char buffer[buffer_size];
    char method[method_size];
    char filename[filename_size];
    
    //读取数据到buffer
    read(client_sock, buffer, sizeof(buffer) - 1);
    
    //判断是否是HTTP请求
    if (!strstr(buffer, "HTTP/")) {
        sendError(sock);
        close(client_sock);
        return;
    }
    
    strcpy(method, strtok(buffer, " /"));
    strcpy(filename, strtok(NULL, " /"));
    
    if (0 == strcmp(filename, "HTTP"))
        strcpy(filename, "index.html");

    if (0 != strcmp(method, "GET")) {
        sendError(sock);
        close(client_sock);
        return;
    }
    
    sendData(sock, filename);
}

//发送数据
void sendData(int *sock, char *filename) {
    int client_sock = *sock;
    char buffer[common_buffer_size];
    char type[common_buffer_size];
    printf("%s\n", filename);
    strcpy(buffer, filename);
    strtok(buffer, ".");
    strcpy(type, strtok(NULL, "."));
    
    //多路选择数据类型，多类型可使用switch代替
    if (0 == strcmp(type, "html")) {
        sendHTML(sock, filename);
    }else if (0 == strcmp(type, "jpg")) {
        sendJPG(sock, filename);
    }else{
        sendError(sock);
        close(client_sock);
        return;
    }
}

//发送页面
void sendHTML(int *sock, char *filename) {
    int client_sock = *sock;
    char buffer[buffer_size];
    FILE *fp;
    
    char status[] = "HTTP/1.1 200 OK\r\n";
    char header[] = "Server: A Simple Web Server\r\nContent-Type: text/html\r\n\r\n";
    
    write(client_sock, status, strlen(status));
    write(client_sock, header, strlen(header));
    
    fp = fopen(filename, "r");
    if (!fp) {
        sendError(sock);
        close(client_sock);
        handleError("open file failed");
        return;
    }
    
    fgets(buffer, sizeof(buffer), fp);
    while(!feof(fp)) {
        write(client_sock, buffer, strlen(buffer));
        fgets(buffer, sizeof(buffer), fp);
    }
    
    fclose(fp);
    close(client_sock);
}

void sendJPG(int *sock, char *filename) {
    int client_sock = *sock;
    char buffer[buffer_size];
    FILE *fp;
    FILE *fw;
    
    // int size = filelength(fileno(fp));

    char status[] = "HTTP/1.1 200 OK\r\n";
    char header[] = "Server: A Simple Web Server\r\nContent-Type: image/jpeg\r\nContent-Length: 657\r\n\r\n";
    
    write(client_sock, status, strlen(status));
    write(client_sock, header, strlen(header));
    fp = fopen(filename, "rb");

    if (NULL == fp) {
        sendError(sock);
        close(client_sock);
        handleError("open file failed");
        return;
    }
    printf("Sending img\n");
    fw = fdopen(client_sock, "w");
    fread(buffer, sizeof(char), sizeof(buffer), fp);
    while (!feof(fp)) {
        fwrite(buffer, sizeof(char), sizeof(buffer), fw);
        // write(client_sock, buffer, strlen(buffer));
        fread(buffer, sizeof(char), sizeof(buffer), fp);
    }
    printf("Finish sending\n");
    // while(!feof(fp)) {
    //     write(client_sock, buffer, strlen(buffer));
    //     fread(buffer, 1, sizeof(buffer), fp);
    // }

    
    fclose(fw);
    fclose(fp);
    close(client_sock);
}

void handleError(const string &msg) {
    cout << msg;
    exit(1);
}

void sendError(int *sock) {
    int client_sock = *sock;
    
    char status[] = "HTTP/1.1 400 Bad Request\r\n";
    char header[] = "Server: A Simple Web Server\r\nContent-Type: text/html\r\n\r\n";
    char body[] = "<html><head><title>Bad Request</title></head><body><p>400 Bad Request</p></body></html>";
    
    write(client_sock, status, strlen(status));
    write(client_sock, header, strlen(header));
    write(client_sock, body, strlen(body));
}