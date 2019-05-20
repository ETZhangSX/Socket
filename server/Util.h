/*
** Util.h
**
*/
#include <cstdio>
#include <cstdlib>
#include <string>
#include <openssl/ssl.h>

#define CHAIN "1_root_bundle.crt"
#define CERTSERVER "2_www.etzhang.xyz.crt"
#define KEYSERVER "3_www.etzhang.xyz.key"
#define CHK_ERR(err, s) if((err) == -1) { perror(s); return -1; }
#define CHK_RV(rv, s) if((rv) != 1) { printf("%s error\n", s); return -1; }
#define CHK_NULL(x, s) if((x) == NULL) { printf("%s error\n", s); return -1; }

ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &isZero);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &outBuffer);
void writeFile(FILE *fp, int *sock);    //边读边写文件
void handle_sigpipe();                  //忽略SIGPIPE信号，防止进程意外关闭
int setSocketNonBlocking(int fd);       //设置套接字为非阻塞模式
void handleError(const std::string &msg);
void setTCPNoDelay(int fd);             //禁用TCP Nagle算法降低延迟
int socket_bind_listen(int port);               //创建绑定并监听套接字

int ssl_read(SSL* ssl, void *buff, size_t n);
int ssl_read(SSL* ssl, std::string &inBuffer, bool &isZero);
int ssl_write(SSL* ssl, void *buff, size_t n);
int ssl_write(SSL* ssl, std::string &outBuffer);
void ssl_writeFile(FILE *fp, SSL* ssl);