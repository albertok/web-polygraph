
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_IOSTREAM_H
#define POLYGRAPH__XSTD_H_IOSTREAM_H

#include "xstd/h/iosfwd.h"

// give priority to newer, backword-incompatible headers
#ifdef HAVE_IOSTREAM
#	include <ios>
#	include <iostream>
#elif HAVE_IOSTREAM_H
#	include <iostream.h>
#endif

#ifdef HAVE_TYPE_IOS_FMTFLAGS
	typedef ios::fmtflags ios_fmtflags;
#elif HAVE_TYPE_IOS_BASE_FMTFLAGS
	typedef std::ios_base::fmtflags ios_fmtflags;
#else
	typedef long ios_fmtflags;
#endif

#endif
