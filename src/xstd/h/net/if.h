
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_NET_IF_H
#define POLYGRAPH__XSTD_H_NET_IF_H

#include "xstd/h/sys/socket.h"
#include "xstd/h/string.h"

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifdef HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif

#if defined(HAVE_WINSOCK2_H)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


#ifndef HAVE_TYPE_IFREQ
#   ifndef IFNAMSIZ
#   define IFNAMSIZ 16
#   endif
	struct ifreq {
		char ifr_name[IFNAMSIZ];
		sockaddr ifr_addr;
	};
#endif

#endif
