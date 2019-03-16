//
//  exam.cpp
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
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;

const int port = 9090;
const int buffer_size = 1<<20;
const int method_size = 1<<10;
const int filename_size = 1<<10;
const int common_buffer_size = 1<<10;

void handleError(const string &msg);
void requestHandling(int *sock);
void sendError(int *sock);
void sendData(int *sock, char *filename);
void sendHTML(int *sock, char *filename);
void sendJPG(int *sock, char *filename);

int main() {
    int server_sock;
    int client_sock;
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    
    socklen_t client_address_size;
    //创建套接字
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    
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
        client_address_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_address_size);
        
        if (client_sock == -1) {
            handleError("accept error");
        }
        
        requestHandling(&client_sock);
    }
    
    close(server_sock);
    return 0;
}

//处理请求
void requestHandling(int *sock) {
    int client_sock = *sock;
    char buffer[buffer_size];
    char method[method_size];
    char filename[filename_size];
    
    read(client_sock, buffer, sizeof(buffer) - 1);
    
    if (!strstr(buffer, "HTTP/")) {
        sendError(sock);
        close(client_sock);
        return;
    }
    
    strcpy(method, strtok(buffer, " /"));
    strcpy(filename, strtok(NULL, " /"));
    
    if (0 != strcmp(method, "GET")) {
        sendError(sock);
        close(client_sock);
        return;
    }
    
    sendData(sock, filename);
}

void sendData(int *sock, char *filename) {
    int client_sock = *sock;
    char buffer[common_buffer_size];
    char type[common_buffer_size];
    
    strcpy(buffer, filename);
    
    strtok(buffer, ".");
    strcpy(type, strtok(NULL, "."));
    
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

void sendHTML(int *sock, char *filename) {
    int client_sock = *sock;
    char buffer[buffer_size];
    FILE *fp;
    
    char status[] = "HTTP/1.0 200 OK\r\n";
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
    
    char status[] = "HTTP/1.0 200 OK\r\n";
    char header[] = "Server: A Simple Web Server\r\nContent-Type: image\r\n\r\n";
    
    write(client_sock, status, strlen(status));
    write(client_sock, header, strlen(header));
    
    fp = fopen(filename, "rb");
    if (NULL == fp) {
        sendError(sock);
        close(client_sock);
        handleError("open file failed");
        return;
    }
    
    fw = fdopen(client_sock, "w");
    fread(buffer, 1, sizeof(buffer), fp);
    while (!feof(fp)) {
        fwrite(buffer, 1, sizeof(buffer), fw);
        fread(buffer, 1, sizeof(buffer), fp);
    }
    
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
    
    char status[] = "HTTP/1.0 400 Bad Request\r\n";
    char header[] = "Server: A Simple Web Server\r\nContent-Type: text/html\r\n\r\n";
    char body[] = "<html><head><title>Bad Request</title></head><body><p>400 Bad Request</p></body></html>";
    
    write(client_sock, status, strlen(status));
    write(client_sock, header, strlen(header));
    write(client_sock, body, strlen(body));
}
