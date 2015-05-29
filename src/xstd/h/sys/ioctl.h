
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_SYS_IOCTL_H
#define POLYGRAPH__XSTD_H_SYS_IOCTL_H

#include "xstd/h/sys/types.h"

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#if !defined(HAVE_IOCTL) && defined(HAVE_IOCTLSOCKET)
	// XXX: use inlined redirector
#	define ioctl ioctlsocket
#endif

#endif
