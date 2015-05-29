
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_DLFCN_H
#define POLYGRAPH__XSTD_H_DLFCN_H

#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifndef HAVE_DLOPEN
	inline void *dlopen(const char *, int) { return 0; }
	inline void *dlsym(void *, const char *) { return 0; }
	inline const char *dlerror() { return "loadable modules not supported"; }
	inline int dlclose(void *) { return -1; }
#endif

#endif
