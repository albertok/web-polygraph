
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_IOMANIP_H
#define POLYGRAPH__XSTD_H_IOMANIP_H

#include <stdlib.h>
#include "xstd/h/iosfwd.h"

#if defined(HAVE_IOMANIP)
#include <iomanip>
#elif defined(HAVE_IOMANIP_H)
#include <iomanip.h>
#endif

// handy manipulators to terminate a program after an error message
inline ostream &xabort(ostream &os) { ::abort(); return os; }
inline ostream &xexit(ostream &os) { ::exit(-1); return os; }

#endif
