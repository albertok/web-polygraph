
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/OLog.h"
#include "base/ObjTimes.h"


void ObjTimes::reset() {
	theLmt = theExp = Time();
	doShowLmt = false;
}

void ObjTimes::store(OLog &log) const {
	log << theLmt << theExp << doShowLmt;
}
