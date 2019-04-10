/*
** Util.h
**
*/
#include <cstdio>
#include <cstdlib>
#include <string>

ssize_t readn(int fd, std::string &inBuffer, bool &isZero);
ssize_t writen(int fd, std::string &outBuffer);
void writeFile(FILE *fp, int *sock);