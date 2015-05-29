
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_SYS_SOCKET_H
#define POLYGRAPH__XSTD_H_SYS_SOCKET_H

#include "xstd/h/sys/types.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif


#ifndef HAVE_TYPE_SOCKLEN_T
	typedef int socklen_t;
#endif


#endif
