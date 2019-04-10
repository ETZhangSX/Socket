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

using namespace std;

const int MAX_BUFFER = 1<<20;

ssize_t readn(int fd, string &inBuffer, bool &isZero) {
	ssize_t n_read = 0;
	ssize_t read_sum = 0;
	char buffer[MAX_BUFFER];

	while (true) {
		if (n_read = read(fd, buffer, MAX_BUFFER) < 0) {
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

    char buffer[buffer_size];

    fw = fdopen(client_sock, "wb");

    while (!feof(fp)) {
        fread(buffer, sizeof(char), sizeof(buffer), fp);
        fwrite(buffer, sizeof(char), sizeof(buffer), fw);
    }

    cout << "Finish sending\n";
}