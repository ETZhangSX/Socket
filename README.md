# HTTP Web Server
[网站主页 www.etzhang.xyz](http://www.etzhang.xyz) (尚未部署)

调试环境：Ubuntu 18.04

部署环境：CentOS 7

## 简介
    - 基于C++的Web服务器，实现了GET请求解析
    - 尚未加入计时器，暂不支持长连接
    - 通过线程池与epoll实现该Web Server，具备一定的并发能力

## 主要技术
    - 使用Epoll的ET模式，非阻塞IO
    - 使用多个线程，每个线程持一个epoll描述符对连接进行管理，并通过线程池管理多个线程避免线程重复创建的开销
    - 主线程只注册Socket监听描述符，负责accept请求，轮询分配注册到子线程的epoll，由子线程负责HTTP请求的解析与响应
    - 使用智能指针代替裸指针，减少内存泄漏可能性

## 模型
