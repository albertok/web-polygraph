
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/CompoundXactStat.h"

CompoundXactStat::CompoundXactStat(): exchanges(0,50) {
	reset();
}

void CompoundXactStat::reset() {
	duration.reset();
	reqSize.reset();
	repSize.reset();
	exchanges.reset();
}

OLog &CompoundXactStat::store(OLog &log) const {
	return log
		<< duration
		<< reqSize
		<< repSize
		<< exchanges
		;
}

ILog &CompoundXactStat::load(ILog &log) {
	return log
		>> duration
		>> reqSize
		>> repSize
		>> exchanges
		;
}

CompoundXactStat &CompoundXactStat::operator +=(const CompoundXactStat &s) {
	duration += s.duration;
	reqSize += s.reqSize;
	repSize += s.repSize;
	exchanges += s.exchanges;
	return *this;
}

bool CompoundXactStat::sane() const {
	return duration.sane() && reqSize.sane() && repSize.sane() &&
		exchanges.sane();
}

void CompoundXactStat::recordCompound(const Time &rptm, const Size &req, const Size &rep, const int ex) {
	duration.record(rptm);
	reqSize.record(req);
	repSize.record(rep);
	exchanges.record(ex);
}

ostream &CompoundXactStat::print(ostream &os, const String &pfx) const {
	duration.print(os, pfx + "rptm.");
	reqSize.print(os, pfx + "req.size.");
	repSize.print(os, pfx + "rep.size.");
	exchanges.print(os, pfx + "exchanges.");
	return os;
}
