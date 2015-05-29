
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <stdlib.h>
#include "xstd/h/math.h"
#include "xstd/Assert.h"
#include "xstd/NetDouble.h"


NetDouble::NetDouble(double x) {
	mnt = (int) floor(0x7fffffff * frexp(x, &exp));
}

NetDouble::operator double() const {
	// avoid floating point exceptions on overflows when we read
	// invalid data
	return abs(exp) <= 1024 ? ldexp(mnt/(double)0x7fffffff, exp) : -1;
}
