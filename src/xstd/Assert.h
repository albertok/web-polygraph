
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_ASSERT_H
#define POLYGRAPH__XSTD_ASSERT_H

#include <stdlib.h>
#include "xstd/Error.h"

// our version of assert(3), mostly for portability purposes
#define Assert(cond) ((cond) ? (void)0 : Abort(__FILE__, __LINE__, #cond))

// same as Assert but calls Exit instead of Abort
#define Check(cond) ((cond) ? (void)0 : Exit(__FILE__, __LINE__, #cond))

// logs current error to cerr and exits if cond fails
#define Must(cond) ((cond) || (Complain(__FILE__, __LINE__), Exit(), false))

// logs current error to cerr if cond fails
#define Should(cond) ((cond) || Complain(__FILE__, __LINE__))


// handy for temporary debugging
#define here __FILE__ << ':' << __LINE__ << ": "

/* internal functions used by macros above */

// logs current err to cerr
extern bool Complain(const char *fname, int lineno);

// aborts program execution and generates coredump
extern void Abort(const char *fname, int lineno, const char *cond);

// aborts program execution without coredump
extern void Exit(const char *fname, int lineno, const char *cond);
extern void Exit();

#endif
