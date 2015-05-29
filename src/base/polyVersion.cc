
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "base/polyVersion.h"

// sync this and remake executables whenever release name changes!
#define POLY_VERSION_STR PACKAGE_VERSION

// for use with ident-like scanners
static String PolyVerIdentStr = 
	"@(#) $Polygraph: " POLY_VERSION_STR " $";

static String PolyCompTimestampIdentStr = 
	"@(#) $CompilationTimestamp: " __DATE__ " " __TIME__ " $";


const String &PolyVersion() {
	Assert(PolyVerIdentStr); // just to use it somewhere
	Assert(PolyCompTimestampIdentStr); // just to use it somewhere
	static String vs = POLY_VERSION_STR;
	return vs;
}

const String &PolyCompDate() {
	static String ds = __DATE__;
	return ds;
}

const String &PolyCompTime() {
	static String ts = __TIME__;
	return ts;
}

const String &PolyLetterhd() {
	static String lh =
		PACKAGE_STRING
		"\n http://www.web-polygraph.org/\n"
		" Copyright 2003-2011 The Measurement Factory.";
	return lh;
}
