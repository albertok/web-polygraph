
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_NETINET_H
#define POLYGRAPH__XSTD_H_NETINET_H

/* there is no standard netinet.h header file, but 
 * the files below are related and often used together */

#include "xstd/h/sys/types.h"
#include "xstd/h/net/if.h" 

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h> /* for TCP_NODELAY */
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NETINET_IN_VAR_H
#include <netinet/in_var.h>
#endif

#if defined(HAVE_WINSOCK2_H) && !defined(_WINSOCK2API_)
#include <winsock2.h>
#endif

// this is needed on Solaris; others?
#ifndef INADDR_NONE
#	ifndef _IN_ADDR_T
#		define _IN_ADDR_T
		typedef unsigned int in_addr_t;
#	endif
#	define INADDR_NONE ((in_addr_t)-1)
#endif

#if !defined(HAVE_INET_MAKEADDR)
	extern in_addr inet_makeaddr(unsigned long net, unsigned long lna);
	extern unsigned long inet_lnaof(in_addr in);
	extern unsigned long inet_netof(in_addr in);
#endif

#endif
