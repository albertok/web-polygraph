
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_SIGNAL_H
#define POLYGRAPH__XSTD_H_SIGNAL_H

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef SIGNAL_HANDLER_TYPE
	typedef SIGNAL_HANDLER_TYPE;
#endif

#ifdef HAVE_SIGNAL
	// some signals may be undefined on some platforms
#	if !defined(SIG_IGN)
#		define SIG_IGN 0
#	endif
#	if !defined(SIGPIPE)
#		define SIGPIPE 0
#	endif
#	if !defined(SIGHUP)
#		define SIGHUP 0
#	endif
#	if !defined(SIGINT)
#		define SIGINT 0
#	endif
#	if !defined(SIGUSR1)
#		define SIGUSR1 0
#	endif
#	if !defined(SIGUSR2)
#		define SIGUSR2 0
#	endif
#else
	inline
	SignalHandler *signal(int, SignalHandler *) {
		return 0;
	}

	// define these constants to avoid #ifdefs in the code
	// if we failed to find signal(), we should not have these pre-defined
#	define SIG_IGN 0
#	define SIGPIPE 0
#	define SIGHUP 0
#	define SIGINT 0
#	define SIGUSR1 0
#	define SIGUSR2 0
#endif


#endif
