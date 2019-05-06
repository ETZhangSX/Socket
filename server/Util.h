/*
** Util.h
**
*/
#include <cstdio>
#include <cstdlib>
#include <string>

ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &isZero);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &outBuffer);
void writeFile(FILE *fp, int *sock);    //边读边写文件
void handle_sigpipe();                  //忽略SIGPIPE信号，防止进程意外关闭
int setSocketNonBlocking(int fd);       //设置套接字为非阻塞模式
void handleError(const string &msg);
void setTCPNoDelay(int fd);             //禁用TCP Nagle算法降低延迟
int socket_bind_listen();               //创建绑定并监听套接字