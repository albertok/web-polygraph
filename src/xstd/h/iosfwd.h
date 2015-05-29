
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_IOSFWD_H
#define POLYGRAPH__XSTD_H_IOSFWD_H

#ifdef HAVE_IOSFWD_H
#	include <iosfwd.h>
#elif HAVE_IOSFWD
#	include <iosfwd>
#else
	class ostream;
	class istream;
#endif

#endif
