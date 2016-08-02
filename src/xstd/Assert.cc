
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/string.h"
#include "xstd/h/iostream.h"

#include "xstd/Assert.h"

bool ComplainSys(const char *fname, int lineno) {
	const Error err = Error::Last();
	cerr << fname << ':' << lineno << ": " << err << endl;
	return false;
}

bool ComplainUs(const char *fname, int lineno, const char *cond) {
	cerr << fname << ':' << lineno << ": soft assertion failed: " << cond << endl;
	return false;
}

bool Complain(const char *fname, int lineno, const char *cond) {
	return Error::Last() ? ComplainSys(fname, lineno) : ComplainUs(fname, lineno, cond);
}

void Abort(const char *fname, int lineno, const char *cond) {
	cerr << fname << ':' << lineno << ": assertion failed: '" 
		<< (cond ? cond : "?") << "'" << endl;
	::abort();
}

void Exit(const char *fname, int lineno, const char *cond) {
	cerr << fname << ':' << lineno << ": assertion failed: '" 
		<< (cond ? cond : "?") << "'" << endl;
	::exit(-2);
}

void Exit() {
	if (const Error err = Error::Last())
		::exit(err.no());
	else
		::exit(-2);
}
