/*
** Main.cpp
** ETZhangSX
*/

#include "EventLoop.h"
#include "Server.h"
#include  <string>

const int port = 80;
const int threadNum = 4;

int main() {
    EventLoop mainLoop;
    Server http_server(&mainLoop, threadNum, port);
    http_server.start();
    mainLoop.loop();
    return 0;
}