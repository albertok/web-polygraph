
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/RangeGenStat.h"

RangeGenStat::RangeGenStat() {
	reset();
}

void RangeGenStat::reset() {
	theOneSize.reset();
	theTotalSize.reset();
	theAbsRelSwapCount = 0;
	theOneOffOverCount = 0;
	theTotalSizeOverCount = 0;
}

OLog &RangeGenStat::store(OLog &log) const {
	return log
		<< theOneSize
		<< theTotalSize
		<< theAbsRelSwapCount
		<< theOneOffOverCount
		<< theTotalSizeOverCount
		;
}

ILog &RangeGenStat::load(ILog &log) {
	return log
		>> theOneSize
		>> theTotalSize
		>> theAbsRelSwapCount
		>> theOneOffOverCount
		>> theTotalSizeOverCount
		;
}

RangeGenStat &RangeGenStat::operator +=(const RangeGenStat &ts) {
	theOneSize += ts.theOneSize;
	theTotalSize += ts.theTotalSize;
	theAbsRelSwapCount += ts.theAbsRelSwapCount;
	theOneOffOverCount += ts.theOneOffOverCount;
	theTotalSizeOverCount += ts.theTotalSizeOverCount;
	return *this;
}

ostream &RangeGenStat::print(ostream &os, const String &pfx) const {
	float n = 0;
	if (theTotalSize.count() != 0)
		n = (float)theOneSize.count()/theTotalSize.count();

	theOneSize.print(os, pfx + "spec_size.");
	theTotalSize.print(os, pfx + "set_size.");
	return os
		<< pfx << "specs_per_set: \t " << n << endl
		<< pfx << "first_last_swap: \t " << theAbsRelSwapCount << endl
		<< pfx << "spec_overflow: \t " << theOneOffOverCount << endl
		<< pfx << "set_overflow: \t " << theTotalSizeOverCount << endl
		;
}
