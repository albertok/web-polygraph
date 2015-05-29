
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_EPOLL_H
#define POLYGRAPH__XSTD_H_EPOLL_H

#ifdef HAVE_EPOLL_CREATE
#include <sys/epoll.h>

inline
int xepoll_create(int size) {
	return ::epoll_create(size);
}

inline
int xepoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
	return ::epoll_ctl(epfd, op, fd, event);
}

inline
int xepoll_wait(int epfd, epoll_event *events, int maxevents, int timeout) {
	return ::epoll_wait(epfd, events, maxevents, timeout);
}

#else

#include "xstd/Assert.h"

// epoll(7) is not supported on this system

#define EPOLLIN 0
#define EPOLLOUT 0
#define EPOLLERR 0
#define EPOLLHUP 0
#define EPOLL_CTL_ADD 0
#define EPOLL_CTL_MOD 0
#define EPOLL_CTL_DEL 0

union epoll_data {
	void *ptr;
	int fd;
	unsigned long u32;
	unsigned long long u64;
};

struct epoll_event {
	unsigned long events;	/* Epoll events */
	epoll_data data;	/* User data variable */
};

inline
int xepoll_create(int) {
	Must(false);
	return 0;
}

inline
int xepoll_ctl(int, int, int, struct epoll_event *) {
	Must(false);
	return 0;
}

inline
int xepoll_wait(int, epoll_event *, int, int) {
	Must(false);
	return 0;
}

#endif


#endif
