
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_SYS_RESOURCE_H
#define POLYGRAPH__XSTD_H_SYS_RESOURCE_H

#include "xstd/Time.h"
#include "xstd/Size.h"
#include "xstd/h/os_std.h"

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>  /* for BSD4_4 to be defined */
#endif

#ifndef HAVE_TYPE_RUSAGE
	// fake structure
	struct rusage {
		timeval ru_stime;
		timeval ru_utime;
		long ru_maxrss;
		long ru_majflt;
		long other[16];
	};
#elif defined(CFG_HOS_SGI) && _ABIAPI
#	define ru_maxrss ru_pad[0]
#	define ru_majflt ru_pad[5]
#endif


// some platforms define maxrss in KBytes, some in pages
inline
Size MaxRss2Size(long maxrss) {
	int kb = -1;
#if defined(CFG_HOS_SGI)
	kb = maxrss;
#elif defined(CFG_HOS_OSF)
	kb = maxrss;
#elif defined(BSD4_4)
	kb = maxrss;
#elif defined(HAVE_GETPAGESIZE)
	kb = (maxrss * ::getpagesize()) / 1024;
#elif defined(PAGESIZE)
	kb = (maxrss * PAGESIZE) / 1024;
#else
	kb = maxrss;
#endif
	return Size::KB(kb);
}


#endif
