
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "base/LevelStat.h"
#include "xstd/gadgets.h"

LevelStat::LevelStat() {
	theIncCnt = theDecCnt = theLevel = 0;
	theSum = Time(0, 0);
	theNom = theDenom = -1;
}

void LevelStat::restart() {
	theIncCnt = theDecCnt = 0; // but keep level
	theStart = theCurStart = TheClock;
	theSum = Time(0, 0);
	theNom = theDenom = -1;
}

void LevelStat::change() {
	if (theStart <= 0)
		theStart = TheClock;
	else
		theSum += (TheClock - theCurStart) * theLevel;
	theCurStart = TheClock;
	theNom = theDenom = -1;
}

double LevelStat::mean() const {
	if (theDenom > 0)
		return theNom/theDenom;

	return theCurStart > theStart && theSum >= 0 ?
		theSum/(theCurStart-theStart) : 
		theLevel;
}

void LevelStat::store(OLog &log) const {
	log << theIncCnt << theDecCnt << theLevel
		<< theStart << theCurStart << theSum;
}

void LevelStat::load(ILog &log) {
	log >> theIncCnt >> theDecCnt >> theLevel
		>> theStart >> theCurStart >> theSum;
}

bool LevelStat::sane() const {
	return theIncCnt >= 0 && theDecCnt >= 0;
}

void LevelStat::keepLevel(const LevelStat &prevLevel) {
	theLevel = prevLevel.theLevel;
}

void LevelStat::merge(const LevelStat &s) {
	if (known() && s.known())
		theLevel += s.theLevel; // imprecise
	else
		theLevel = s.theLevel;
	LevelStat::join(s);
}

void LevelStat::concat(const LevelStat &s) {
	if (s.known())
		theLevel = s.theLevel;
	// else should not happen
	LevelStat::join(s);
}

void LevelStat::join(const LevelStat &s) {
	if (theStart < 0) {
		// join() should not alter theLevel that can be already computed
		const int savedLevel = theLevel;
		*this = s;
		theLevel = savedLevel;
	} else
	if (theStart >= 0 && s.theStart >= 0) {
		theIncCnt += s.theIncCnt;
		theDecCnt += s.theDecCnt;

		// set theNom before changing members nom() depends on
		theNom = nom() + s.nom();

		// note: this will produce correct means for levels
		// with different start/stop times, but usually
		// merged_mean will not be (mean1 + mean2)/2.
		theStart = Min(theStart, s.theStart);
		// assume that the curStart is also the end
		theCurStart = Max(theCurStart, s.theCurStart);
		theSum += s.theSum;

		theDenom = (theCurStart-theStart).secd();
	} 
	// else do nothing, should not happen
}

double LevelStat::nom() const {
	return theNom > 0 ? theNom : theSum.secd();
}

double LevelStat::denom() const {
	return theDenom > 0 ? theDenom : (theCurStart-theStart).secd();
}

ostream &LevelStat::print(ostream &os, const String &pfx) const {
	os << pfx << "started:   \t " << incCnt() << endl;
	os << pfx << "finished:  \t " << decCnt() << endl;
	os << pfx << "level.mean:\t " << mean() << endl;
	os << pfx << "level.last:\t " << level() << endl;
	return os;
}
