
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_SYS_TYPES_H
#define POLYGRAPH__XSTD_H_SYS_TYPES_H

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif


#ifndef HAVE_TYPE_RLIM_T
	typedef int rlim_t;
#endif


#endif
