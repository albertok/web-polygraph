
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_XPORT_H
#define POLYGRAPH__XSTD_XPORT_H

#include "xstd/h/time.h"

// here we put various x-wrappers that exist only because some environments
// do not implement corresponding functions [correctly].


extern time_t xtimegm(struct tm *t);


#endif
