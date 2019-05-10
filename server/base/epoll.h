/*
** epoll.h
** ETZhangSX
** Rewrite epoll.h to adapt to MAC OSX
*/
#pragma once
#if !defined(__APPLE__) && !defined(__FreeBSD__)

#include <sys/epoll.h>

#else

#include <stdint.h>
#include <sys/event.h>

enum EPOLL_EVENTS {
	EPOLLIN = 0X001,
	EPOLLPRI = 0X002,
	EPOLLOUT = 0X004,

	EPOLLERR = 0X008,
	EPOLLHUP = 0X010,

    EPOLLET = 0X020,
    EPOLLLT = 0X040,

    EPOLLRDNORM = 0X40,
    EPOLLWRNORM = 0X004,
};

enum EPOLL_FLAG {
	EPOLL_CLOEXEC = 0X001,
};

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

typedef union epoll_data {
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;

} epoll_data_t;

struct epoll_event {
	uint32_t events;
	epoll_data_t data;
};

struct epoll_res {
	int size;
	struct epoll_event *events;
	struct kevent *eventlist;
};

const int _EPOLL_SIZE = 1024 * 10;

int 	epoll_wait1( int epfd,struct epoll_res *events,int maxevents,int timeout );
int 	epoll_wait( int epfd,struct epoll_event *events,int maxevents,int timeout );
int 	epoll_ctl( int epfd,int op,int fd,struct epoll_event * );
int 	epoll_create( int size );
int		epoll_create1( int flag );
#endif