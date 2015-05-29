
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "base/StatIntvlRec.h"
#include "loganalyzers/LoadStex.h"

LoadStex::LoadStex(const String &aKey, const String &aName):
	theKey(aKey), theName(aName) {
}

double LoadStex::perDuration(double meas, const StatIntvlRec &rec) const {
	return rec.theDuration > 0 ? Ratio(meas, rec.theDuration.secd()) : -1;
}
