
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_LIBINIT_H
#define POLYGRAPH__XSTD_LIBINIT_H


// declares a standard library initializer

#define LIB_INITIALIZER(Name)\
static \
class Name { \
	public: \
		Name() { if (!TheUseCount++) init(); } \
		~Name() { if (!--TheUseCount) clean(); } \
\
	protected: \
		void init(); \
		void clean(); \
\
	protected: \
		static int TheUseCount; \
} The ## Name;

#endif
