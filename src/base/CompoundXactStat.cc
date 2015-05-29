
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

ostream &CompoundXactStat::print(ostream &os, const String &pfx) const {
	duration.print(os, pfx + "duration.");
	reqSize.print(os, pfx + "req_size.");
	repSize.print(os, pfx + "rep_size.");
	exchanges.print(os, pfx + "exchanges.");
	return os;
}
