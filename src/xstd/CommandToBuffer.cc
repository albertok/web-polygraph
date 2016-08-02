
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <stdio.h>
#include "xstd/CommandToBuffer.h"
#include "xstd/String.h"


#if !defined(HAVE_POPEN) && defined(HAVE__POPEN)
	inline FILE *popen(const String &cmd, const char *mode) { return _popen(cmd, mode); }
#endif
#if !defined(HAVE_PCLOSE) && defined(HAVE__PCLOSE)
	inline int pclose(FILE *f) { return _pclose(f); }
#endif


stringstream *xstd::CommandToBuffer(const String &cmd) {
	// unfortunately, pipes are not yet standard in iostream lib
	// we emulate similar functionality by using stdio calls

	// open the pipe to cmd
	FILE *const fp = ::popen(cmd.cstr(), "r");
	if (!fp)
		return 0;

	stringstream *const sbuf = new stringstream;
	int c;
	while ((c = fgetc(fp)) != EOF)
		*sbuf << (char)c;

	// close the pipe
	const bool wasError = ferror(fp);
	if (pclose(fp) || wasError) {
		delete sbuf;
		return 0;
	}

	return sbuf;
}
