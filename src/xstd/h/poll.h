
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_POLL_H
#define POLYGRAPH__XSTD_H_POLL_H

#ifdef HAVE_POLL
#include <poll.h>

/* Linux does not define "NORM" */
#ifndef POLLRDNORM
#define POLLRDNORM POLLIN
#endif
#ifndef POLLWRNORM
#define POLLWRNORM POLLOUT
#endif

inline
int xpoll(pollfd *fds, unsigned int nfds, int timeout) {
	return ::poll(fds, nfds, timeout);
}


#else

#include "xstd/Assert.h"

#define POLLRDNORM 1
#define POLLWRNORM 2
#define POLLERR 4
#define POLLHUP 8
#define POLLNVAL 16

struct pollfd { int fd;  short events; short revents; };

inline
int xpoll(pollfd *, unsigned int, int) {
	Must(false); // poll(2) is not supported on this system
	return 0;
}

#endif


#endif
