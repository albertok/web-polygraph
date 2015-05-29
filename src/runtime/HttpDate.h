
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HTTPDATE_H
#define POLYGRAPH__RUNTIME_HTTPDATE_H

#include "xstd/h/iosfwd.h"


// all HTTP date routines follow HTTP/1.1 date format requirements

extern Time HttpDateParse(const char *date, int len);
extern ostream &HttpDatePrint(ostream &os, Time date);

// prints (possibly cached) curent time
extern ostream &HttpDatePrint(ostream &os);

// all HTTP date calculations must use rounded-to-the-sec timestamps
inline
Time HttpDateAtLeast(const Time &precise) {
	return Time::Sec(precise.sec() + (precise.msec() > 0));
}

inline
Time HttpDateAtMost(const Time &precise) {
	return Time::Sec(precise.sec());
}

#endif
