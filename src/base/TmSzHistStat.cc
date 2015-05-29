
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/TmSzHistStat.h"


TmSzHistStat::TmSzHistStat() {
	reset();
}

void TmSzHistStat::reset() {
	theTm.reset();
	theSz.reset();
}

OLog &TmSzHistStat::store(OLog &log) const {
	return log << theTm << theSz;
}

ILog &TmSzHistStat::load(ILog &log) {
	return log >> theTm >> theSz;
}

TmSzHistStat &TmSzHistStat::operator +=(const TmSzHistStat &ts) {
	theTm += ts.theTm;
	theSz += ts.theSz;
	return *this;
}

ostream &TmSzHistStat::print(ostream &os, const String &pfx) const {
	theTm.print(os, pfx + "rptm.");
	theSz.print(os, pfx + "size.");
	return os;
}
