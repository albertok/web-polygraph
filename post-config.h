
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

/* Fine-tune ./configure-generated configuration. */

#ifndef POLYGRAPH__POST_CONFIG_H
#define POLYGRAPH__POST_CONFIG_H

using namespace std;

/* various post-processing fixes and adjustments */

// increase FD_SETSIZE if needed and OS supports FD_SETSIZE redifinition
#if !defined(__linux__) && !defined(linux)
#	if PROBED_MAXFD > 0 && DEFAULT_FD_SETSIZE > 0 && PROBED_MAXFD > DEFAULT_FD_SETSIZE
#		ifdef FD_SETSIZE
#			warning FD_SETSIZE is already defined
#		else
#			define FD_SETSIZE PROBED_MAXFD
#		endif
#	endif
#endif

// what file scanner (select(2), poll(2), epoll(7), etc.) use by default
// note: our current heuristic will fail in many environments
#if defined(HAVE_EPOLL_CREATE)
#	define PG_PREFFERED_FILE_SCANNER Epoll
#elif defined(HAVE_POLL)
#	define PG_PREFFERED_FILE_SCANNER Poll
#else
#	define PG_PREFFERED_FILE_SCANNER Select
#endif

// define CONFIG_HOST_OS_* macros (shortened to CFG_HOS_*)
// this code is similar to Squid's include/config.h.in
#if defined(__sun__) || defined(__sun)                
#	define CFG_HOS_SUN
#	if defined(__SVR4)                                  
#		define CFG_HOS_SOLARIS
#	else                                                
#		define CFG_HOS_SUNOS
#	endif
#elif defined(__sgi__) || defined(sgi) || defined(__sgi) 
#	define CFG_HOS_SGI
#	if !defined(_SVR4_SOURCE)
#		define _SVR4_SOURCE                
#	endif
#elif defined(__hpux)                           
#	define CFG_HOS_HPUX
#	define CFG_HOS_SYSV
#elif defined(__osf__)                          
#	define CFG_HOS_OSF
#elif defined(__ultrix)                                
#	define CFG_HOS_ULTRIX
#elif defined(_AIX)                                  
#	define CFG_HOS_AIX
#elif defined(__linux__)                        
#	define CFG_HOS_LINUX
#elif defined(__FreeBSD__)                        
#	define CFG_HOS_FREEBSD
#elif defined(__NeXT__)
#	define CFG_HOS_NEXT
#elif defined(__bsdi__)
#	define CFG_HOS_BSDI                         
#elif defined(__NetBSD__)
#	define CFG_HOS_NETBSD
#elif defined(__CYGWIN32__) || defined(WIN32) || defined(WINNT) || defined(__WIN32__) || defined(__WIN32)
#	define CFG_HOS_MSWIN
#elif defined(sony_news) && defined(__svr4)
#	define CFG_HOS_NEWSOS6
#elif defined(__EMX__) || defined(OS2) || defined(__OS2__)
#	define CFG_HOS_OS2
#endif

#endif /* POLYGRAPH__POST_CONFIG_H */
