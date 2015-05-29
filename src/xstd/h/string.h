
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_STRING_H
#define POLYGRAPH__XSTD_H_STRING_H

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#if !defined(HAVE_STRCASECMP) && defined(HAVE_STRICMP)
	inline
	int strcasecmp(const char *s1, const char *s2) {
		return stricmp(s1, s2);
	}
#endif

#if !defined(HAVE_STRNCASECMP) && defined(HAVE_STRNICMP)
	inline
	int strncasecmp(const char *s1, const char *s2, size_t sz) {
		return strnicmp(s1, s2, sz);
	}
#endif

#endif
