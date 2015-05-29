
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_TIME_H
#define POLYGRAPH__XSTD_H_TIME_H

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if !defined(HAVE_TIMEVAL_T) && defined(HAVE_WINSOCK2_H) && !defined(_WINSOCK2API_)
#include <winsock2.h>
#endif


#if defined(HAVE_EXTERN_TIMEZONE)
	extern time_t timezone;
#endif

// define xtimezone to be whatever ./configure told us
#if defined(HAVE_TIMEZONE)
	inline time_t xtimezone() { return HAVE_TIMEZONE; }
#else
	inline time_t xtimezone() { return 0; }
#endif


#endif
