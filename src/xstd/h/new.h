
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_NEW_H
#define POLYGRAPH__XSTD_H_NEW_H

#include <new>

typedef void (*xnew_handler)();

inline bool xset_new_handler(xnew_handler h) {
	// VC++ has deliberately broken set_new_handler
	// and a different interface for _set_new_handler
#if defined(HAVE__SET_NEW_HANDLER)
	//_set_new_handler(h);
	return false;
#elif defined(HAVE_SET_NEW_HANDLER)
	set_new_handler(h);
	return true;
#else
	return false;
#endif
}

#endif
