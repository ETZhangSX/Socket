 set(SRCS
    Channel.cpp
    Epoll.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    HttpServer.cpp
    Main.cpp
    Server.cpp
    ThreadPool.cpp
    Util.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/base)


add_executable(HttpServer ${SRCS})
target_link_libraries(HttpServer libserver_base)


add_subdirectory(base)