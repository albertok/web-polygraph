
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/TmSzStat.h"


TmSzStat::TmSzStat() {
	reset();
}

TmSzStat::TmSzStat(const AggrStat &aTm, const AggrStat &aSz): theTm(aTm), theSz(aSz) {
}

void TmSzStat::reset() {
	theTm.reset();
	theSz.reset();
}

bool TmSzStat::active() const {
	return theTm.count() > 0 || theSz.count() > 0;
}

OLog &TmSzStat::store(OLog &log) const {
	return log << theTm << theSz;
}

ILog &TmSzStat::load(ILog &log) {
	return log >> theTm >> theSz;
}

TmSzStat &TmSzStat::operator +=(const TmSzStat &ts) {
	theTm += ts.theTm;
	theSz += ts.theSz;
	return *this;
}

ostream &TmSzStat::print(ostream &os, const String &pfx) const {
	theTm.print(os, pfx + "rptm.");
	theSz.print(os, pfx + "size.");
	return os;
}
