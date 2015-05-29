
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_OS_STD_H
#define POLYGRAPH__XSTD_H_OS_STD_H

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

#if defined(HAVE_WINBASE_H)
#	include <winsock2.h>
#	include <winbase.h>
#endif

#if defined(HAVE_WINBASE_H) && !defined(HAVE_SLEEP)
	inline int sleep(int secs) { Sleep(1000*secs); return 0; }
#endif

#if defined(HAVE_WINBASE_H) && !defined(HAVE_UNLINK)
	inline int unlink(const char *fname) { return DeleteFile(fname); }
#endif

#endif
