
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_REGEX_H
#define POLYGRAPH__XSTD_H_REGEX_H

#if defined(HAVE_REGEX_H)
#	include "xstd/h/sys/types.h"
#	include <regex.h>
#else
	struct regex_t {};
	inline int regcomp(regex_t *, const char *, int) { return -1; }
	inline int regexec(const regex_t *, const char *, int, void*, int) { return -1; }
	inline int regerror(int, const regex_t *, char *, int) { return -1; }
	inline void regfree(regex_t *) {}
#endif

#endif
