
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "client/SessionStats.h"


SessionStats::SessionStats() {
	restart();
}

void SessionStats::restart() {
	theStart = TheClock;
	theXactCnt = 0;
	theFillSz = 0;
	theFillCnt = 0;
	theXactErrCnt = 0;
}

Time SessionStats::duration() const {
	Assert(theStart > 0);
	return TheClock - theStart;
}

Counter SessionStats::xactCnt() const {
	return theXactCnt;
}

BigSize SessionStats::fillSz() const {
	return theFillSz;
}

Counter SessionStats::fillCnt() const {
	return theFillCnt;
}

Counter SessionStats::xactErrCnt() const {
	return theXactErrCnt;
}
