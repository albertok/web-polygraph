
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "xstd/FileScanner.h"
#include "base/UniqId.h"
#include "runtime/globals.h"

String ThePrgName = "?";
FileScanner *TheFileScanner = 0;
UniqId TheGroupId;

/* initialization */

int GlobInit::TheUseCount = 0;

void GlobInit::init() { 
	TheFileScanner = 0; // selected run-time
}

void GlobInit::clean() {
	delete TheFileScanner; TheFileScanner = 0;
}
