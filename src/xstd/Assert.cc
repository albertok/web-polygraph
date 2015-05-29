
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/string.h"
#include "xstd/h/iostream.h"

#include "xstd/Assert.h"

bool Complain(const char *fname, int lineno) {
	cerr << fname << ':' << lineno << ": " << Error::Last() << endl;
	return false;
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
