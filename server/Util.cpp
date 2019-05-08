/*
** Util.cpp
**
*/

#include "Util.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <iostream>

using namespace std;
const int MAX_BUFFER = 1<<20;

ssize_t readn(int fd, void *buff, size_t n) {
	size_t nleft = n;
	ssize_t n_read = 0;
	ssize_t read_sum = 0;
	char *buffer = (char*) buff;

	while (nleft > 0) {
		if ((n_read = read(fd, buffer, nleft)) < 0) {
			if (errno == EINTR)
				n_read = 0;
			else if (errno == EAGAIN) {
				return read_sum;
			}
			else {
				return -1;
			}
		}
		else if (n_read == 0) {
			break;
		}
		read_sum += n_read;
		nleft -= n_read;
		buffer += n_read;
	}
	return read_sum;
}

ssize_t readn(int fd, string &inBuffer, bool &isZero) {
	ssize_t n_read = 0;
	ssize_t read_sum = 0;
	char buffer[MAX_BUFFER];

	while (true) {
		if ((n_read = read(fd, buffer, MAX_BUFFER)) < 0) {
			if (errno == EINTR) {
				continue;
			}
			else if (errno == EAGAIN) {
				return read_sum;
			}
			else {
				perror("read error");
				return -1;
			}
		}
		else if (n_read == 0) {
			isZero = true;
			break;
		}
		read_sum += n_read;
		inBuffer += string(buffer, buffer + n_read);
	}

	return read_sum;
}

ssize_t writen(int fd, void *buff, size_t n) {
	size_t nleft = n;
	ssize_t nwritten = 0;
	ssize_t write_sum = 0;

	char *buffer = (char *) buff;

	while (nleft > 0) {
		if (( nwritten = write(fd, buffer, nleft)) <= 0) {
			if (nwritten < 0) {
				if (errno == EINTR) {
					nwritten = 0;
					continue;
				}
				else if (errno == EAGAIN) {
					return write_sum;
				}
				else return -1;
			}
		}
		write_sum += nwritten;
		nleft -= nwritten;
		buffer += nwritten;
	}
	return write_sum;
}

ssize_t writen(int fd, string &outBuffer) {
	size_t remained_len = outBuffer.size();
	ssize_t n_write = 0;
	ssize_t write_sum = 0;
	const char *ptr = outBuffer.c_str();
	while (remained_len > 0) {
		if ((n_write = write(fd, ptr, remained_len)) <= 0) {
			if (n_write < 0) {
				if (errno == EINTR) {
					n_write = 0;
					continue;
				}
				else if (errno == EAGAIN) {
					break;
				}
				else {
					return -1;
				}
			}
		}
		write_sum += n_write;
		remained_len -= n_write;
		ptr += n_write;
	}
	if (write_sum == static_cast<int>(outBuffer.size())) {
		outBuffer.clear();
	}
	else {
		outBuffer = outBuffer.substr(write_sum);
	}
	return write_sum;
}

// 边读边发文件
void writeFile(FILE *fp, int *sock) {
	int client_sock = *sock;
    FILE *fw;

    char buffer[MAX_BUFFER];

    fw = fdopen(client_sock, "wb");

    while (!feof(fp)) {
        fread(buffer, sizeof(char), sizeof(buffer), fp);
        fwrite(buffer, sizeof(char), sizeof(buffer), fw);
    }
    cout << "Finish sending\n";
}

//忽略SIGPIPE防止进程意外退出
void handle_sigpipe() {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if (sigaction(SIGPIPE, &sa, NULL))
		return;
}

//设置socket为非阻塞模式
int setSocketNonBlocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);
	if (flag == -1) {
		cout << "GETFL Failed\n";
		return -1;
	}
	flag |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flag) == -1) {
		cout << "SETFL Failed\n";
		return -1;
	}
	return 0;
}

void handleError(const string &msg) {
    cout << msg;
    // exit(1);
}

void setTCPNoDelay(int fd) {
	int enable = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}

int socket_bind_listen(int port) {
    int listenFd;
    struct sockaddr_in listen_addr;

    listenFd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenFd == -1) {
        handleError("socket error");
		return -1;
    }

    int optval = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        handleError("setsocketopt error");
		return -1;
    }

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(port);

    if (bind(listenFd, (struct sockaddr*) &listen_addr, sizeof(listen_addr)) == -1) {
        handleError("bind error");
		return -1;
    }

    if (listen(listenFd, 512) == -1) {
        handleError("listen error");
		return -1;
    }
	return listenFd;
}