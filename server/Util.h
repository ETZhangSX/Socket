/*
** Util.h
**
*/
#include <cstdlib>
#include <string>

ssize_t readn(int fd, std::string &inBuffer, bool &isZero);
ssize_t writen(int fd, std::string &outBuffer);