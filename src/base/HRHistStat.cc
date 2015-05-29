
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/HRHistStat.h"

#include "xstd/gadgets.h"


HRHistStat::HRHistStat() {
	reset();
}

void HRHistStat::reset() {
	theHits.reset();
	theMisses.reset();
}

OLog &HRHistStat::store(OLog &log) const {
	return log << theHits << theMisses;
}

ILog &HRHistStat::load(ILog &log) {
	return log >> theHits >> theMisses;
}

HRHistStat &HRHistStat::operator +=(const HRHistStat &hr) {
	theHits += hr.theHits;
	theMisses += hr.theMisses;
	return *this;
}

ostream &HRHistStat::print(ostream &os, const String &hit, const String &miss, const String &pfx) const {
	theHits.print(os, pfx + hit + ".");
	theMisses.print(os, pfx + miss + ".");
	return os;
}
