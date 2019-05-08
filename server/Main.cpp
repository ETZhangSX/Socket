/*
** Main.cpp
** ETZhangSX
*/

#include "EventLoop.h"
#include "Server.h"
#include  <string>
#include <iostream>

using namespace std;

const int port = 80;
const int threadNum = 4;

int main() {
    cout << "port:\t" << port << endl;;
    cout << "Creating mainLoop\n";
    EventLoop mainLoop;
    cout << "Created successfully\n";
    cout << "Creating server\n";
    Server http_server(&mainLoop, threadNum, port);
    cout << "Created successfully\n";
    cout << "Starting server\n";
    http_server.start();
    cout << "Started successfully\n";
    cout << "Starting loop\n";
    mainLoop.loop();
    return 0;
}
