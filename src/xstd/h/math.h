
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_MATH_H
#define POLYGRAPH__XSTD_H_MATH_H

#include <math.h>

#ifndef HAVE_RINT
	inline double rint(double x) { return (int)(x + 0.5); }
#endif


#endif
