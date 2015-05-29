
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/math.h"

#include "runtime/StatPhase.h"
#include "runtime/LogComment.h"
#include "runtime/TransFactor.h"

TransFactor::TransFactor(const StatPhase *aPhase, const String &aName): 
	thePhase(aPhase), theName(aName), theBeg(1), theEnd(1), theK(0) {
	Assert(thePhase);
}

// note: simply left some values unchanged on failure
// the caller will check and complain
void TransFactor::configure(double beg, double end, const TransFactor *prev) {
	if (beg >= 0)
		theBeg = beg;
	else
	if (prev) // use the previous end factor if needed
		theBeg = prev->theEnd;

	theEnd = end >= 0 ? end : theBeg;	

	const double fdiff = theEnd - theBeg;
	
	if (fdiff && !goal())
		return;

	// cache often needed value
	// note: duration takes priority over xact count if both are given
	theK = 0;
	if (goal().duration() > 0)
		theK = fdiff/goal().duration().secd();
	else
	if (goal().xactCnt() > 0)
		theK = fdiff/goal().xactCnt();
	else
	if (goal().fillSz() > 0)
		theK = fdiff/goal().fillSz().byted();
	else
		theK = 0;
	Should((fdiff != 0) == (theK != 0));
}

const GoalRec &TransFactor::goal() const {
	return thePhase->goal();
}

double TransFactor::current() const {
	const double t = 
		goal().duration() > 0 ? thePhase->duration().secd() : 
		(goal().xactCnt() > 0 ? (double)thePhase->xactCnt() : 
		thePhase->fillSz().byted());		 

	const double f = theBeg + theK*t;
	const double d = f*f + 4*theK;
	const double newF = d >= 0 ? (sqrt(d) + f)/2 : (f/2);

	// make sure load factor stays between beg and end
	return theBeg <= theEnd ?
		MiniMax(theBeg, newF, theEnd):
		MiniMax(theEnd, newF, theBeg);
}

void TransFactor::changeBy(double delta) {
	if (flat()) {
		change(theBeg, theBeg + delta*theBeg, "level");
		theEnd = theBeg;
	} else {
		change(theK, theK + delta*theK, "slope");
	}
}

void TransFactor::setTo(double level) {
	if (flat()) {
		change(theBeg, level, "level");
		theEnd = theBeg;
	} else {
		change(theK, level, "slope");
	}
}

void TransFactor::change(double &curVal, double newVal, const char *kind) {
	if (newVal < 0) {
		Comment(6) << theName << " factor cannot become negative, "
			"using 0% instead" << endc;
		newVal = 0;
	} else if (newVal > 1) {
		Comment(6) << theName << " factor cannot exceed 100%, using "
			"100% instead" << endc;
		newVal = 1;
	}

	Comment(6) << "fyi: changing " << theName << " factor " << kind <<
		" from " << (100*curVal) << "% to " << (100*newVal) << '%' << endc;
	curVal = newVal;
}
