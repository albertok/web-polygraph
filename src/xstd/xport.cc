
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"
#include "xstd/xport.h"

// XXX: move xport. stuff to Time.

#ifndef HAVE_TIMEGM
	static
	time_t fake_timegm(struct tm *t) {
		const time_t res = mktime(t);

#	if defined(HAVE_TM_GMTOFF)
		const struct tm *local = localtime(&res);
		return res + local->tm_gmtoff;
#	elif defined(HAVE_TIMEZONE)
		const time_t dst = t->tm_isdst > 0 ? -3600 : 0;
		return res - (xtimezone() + dst);
#	else
		return res;
#	endif
	}
#endif

time_t xtimegm(struct tm *t) {
#	ifdef HAVE_TIMEGM
		return timegm(t);
#	else
		return fake_timegm(t);
#	endif
}
