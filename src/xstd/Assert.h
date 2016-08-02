
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
#define Must(cond) ((cond) || (Complain(__FILE__, __LINE__, #cond), Exit(), false))

// logs current error to cerr if cond fails
#define Should(cond) ((cond) || Complain(__FILE__, __LINE__, #cond))

// For system and library calls: Logs Error::Last() to cerr if condition fails
#define ShouldSys(cond) ((cond) || Complain(__FILE__, __LINE__, #cond))
// For internal logic bugs: Logs condition to cerr if condition fails
#define ShouldUs(cond) ((cond) || ComplainUs(__FILE__, __LINE__, #cond))


// handy for temporary debugging
#define here __FILE__ << ':' << __LINE__ << ": "

/* internal functions used by macros above */

// logs the last known system or library error if available;
// otherwise logs the condition, treating its failure as a bug
extern bool Complain(const char *fname, int lineno, const char *cond);
// logs supplied condition, treating its failure as a bug
extern bool ComplainUs(const char *fname, int lineno, const char *cond);

// aborts program execution and generates coredump
extern void Abort(const char *fname, int lineno, const char *cond);

// aborts program execution without coredump
extern void Exit(const char *fname, int lineno, const char *cond);
extern void Exit();

#endif
