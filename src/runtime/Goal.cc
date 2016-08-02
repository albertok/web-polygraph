
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "xstd/gadgets.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "runtime/StatPhase.h"
#include "runtime/Goal.h"

#include "pgl/GoalSym.h"

Counter Goal::MinXactForRatios = 1000; // XXX: magic constant


Goal::Goal() {
}

void Goal::configure(const GoalSym &cfg) {
	theDuration = cfg.duration();
	cfg.xactCount(theXactCnt);
	cfg.fillSize(theFillSz);
	cfg.errs(theErrs.theRatio, theErrs.theCount);
}

bool Goal::reached(const GoalSubj &s) const {
	return reachedPositive(s) || reachedNegative(s);
}

bool Goal::reachedPositive(const GoalSubj &s) const {
	return // meeting one sub-goal is enough!
		(theDuration >= 0 && theDuration <= s.duration()) ||
		(theXactCnt >= 0 && theXactCnt <= s.xactCnt()) ||
		(theFillSz >= 0 && theFillSz <= s.fillSz())
		;
}

bool Goal::reachedNegative(const GoalSubj &s) const {
	return
		(theErrs.count() >= 0 && theErrs.count() <= s.xactErrCnt()) ||
		(s.xactCnt() >= MinXactForRatios && theErrs.ratio() >= 0 && theErrs.ratio() <= s.xactErrRatio())
		;
}

template <class T>
void GoalProgressItem(ostream &os, const char *label, const T &cur, const T &goal) {

	const ios_fmtflags flags = os.setf(ios::left);
	os << '\t' << setw(16) << label << ' ' << cur;
	os.flags(flags);

	if (goal >= 0) {
		const double p = cur*100.0/goal;
		os << " goal: " << goal << " (" << p << "% complete)";
	}

	os << endl;
}

void Goal::reportProgress(ostream &os, const GoalSubj &s) const {
	GoalProgressItem(os, "duration:" , s.duration(), theDuration);
	GoalProgressItem(os, "xact.count:", s.xactCnt(), theXactCnt);
	GoalProgressItem(os, "fill.size:", s.fillSz(), theFillSz);
	GoalProgressItem(os, "xact.errs.count:", s.xactErrCnt(), theErrs.count());
	GoalProgressItem(os, "xact.errs.ratio:", s.xactErrRatio(), theErrs.ratio());

	if (theErrs.ratio() >= 0) {
		if (s.xactCnt() < MinXactForRatios)
			os << "\t# ratios not checked until first "
				<< MinXactForRatios << " xactions" << endl;
	}
}

/* GoalSubj */

GoalSubj::~GoalSubj() {
}

double GoalSubj::xactErrRatio() const {
	return Ratio(xactErrCnt(), xactCnt());
}
