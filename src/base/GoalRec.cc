
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/GoalRec.h"
#include "xstd/gadgets.h"


template <class T>
inline
void SetOrInc(T &var, const T &inc) {
	if (var < 0)
		var = inc;
	else
	if (inc >= 0)
		var += inc;
}


/* ErrGoalRec */

ErrGoalRec::ErrGoalRec(): theRatio(-1), theCount(-1) {
}

ErrGoalRec &ErrGoalRec::operator +=(const ErrGoalRec &e) {

	SetOrInc(theCount, e.theCount);

	if (theRatio < 0)
		theRatio = e.theRatio;
	else
	if (e.theRatio >= 0)
		theRatio = Min(theRatio, e.theRatio);

	return *this;
}

OLog &ErrGoalRec::store(OLog &ol) const {
	return ol << theRatio << theCount;
}

ILog &ErrGoalRec::load(ILog &il) {
	return il >> theRatio >> theCount;
}

ostream &ErrGoalRec::print(ostream &os) const {
	if (theRatio > 0)
		return os << 100*theRatio << '%';
	if (theCount > 0)
		return os << theCount;
	return os << "<none>";
}

ostream &ErrGoalRec::print(ostream &os, const String &pfx) const {
	if (theRatio > 0)
		os << pfx << "ratio:\t " << 100*theRatio << endl;
	if (theCount > 0)
		os << pfx << "count:\t " << theCount << endl;
	return os;
}

/* GoalRec */

GoalRec::GoalRec(): theXactCnt(-1), theFillSz(-1) {
}

GoalRec::operator void*() const {
	// note: errors alone do not count as a real goal
	return theDuration >= 0 || theXactCnt >= 0 || theFillSz >= 0 ?
		(void*)-1 : 0;
}

void GoalRec::join(const GoalRec &g) {
	if (theDuration < 0)
		theDuration = g.theDuration;
	SetOrInc(theXactCnt, g.theXactCnt);
	SetOrInc(theFillSz, g.theFillSz);
	theErrs += g.theErrs;
}

void GoalRec::concat(const GoalRec &g) {
	SetOrInc(theDuration, g.theDuration);
	join(g);
}

void GoalRec::merge(const GoalRec &g) {
	join(g);
}

OLog &GoalRec::store(OLog &ol) const {
	return ol << theDuration << theXactCnt << theFillSz << theErrs;
}

ILog &GoalRec::load(ILog &il) {
	return il >> theDuration >> theXactCnt >> theFillSz >> theErrs;
}

ostream &GoalRec::print(ostream &os) const {
#define grp_stream_hack  (count++ ? os << ':' : os)
	int count = 0;

	if (theDuration >= 0)
		grp_stream_hack << theDuration;
	if (theXactCnt >= 0)
		grp_stream_hack << theXactCnt;
	if (theFillSz >= 0)
		grp_stream_hack << theFillSz;
	if (theErrs)
		theErrs.print(grp_stream_hack);

	if (!count)
		os << "<none>";

	return os;
}


ostream &GoalRec::print(ostream &os, const String &pfx) const {

	if (theDuration >= 0)
		os << pfx << "duration:\t " << theDuration.secd() << endl;
	if (theXactCnt >= 0)
		os << pfx << "xactions:\t " << theXactCnt << endl;
	if (theFillSz >= 0)
		os << pfx << "fill.size:\t " << theFillSz.byted() << endl;
	if (theErrs)
		theErrs.print(os, pfx + "errors.");
	return os;
}
