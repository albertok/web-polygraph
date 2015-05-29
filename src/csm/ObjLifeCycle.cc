
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"

#include "xstd/Clock.h"
#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "base/RndPermut.h"
#include "base/ObjTimes.h"
#include "pgl/PglQualifSym.h"
#include "pgl/ObjLifeCycleSym.h"
#include "csm/ObjLifeCycle.h"



ObjLifeCycle::ObjLifeCycle():
	theModTime(0), theExpSelector(0),
	theModVar(0), theShowModRatio(0) {
}

ObjLifeCycle::~ObjLifeCycle() {
	delete theExpSelector;
	// do not delete PGL distributions because they are not cloned
	// delete theModTime;
}

void ObjLifeCycle::configure(const ObjLifeCycleSym *cfg) {
	Assert(cfg);

	if (cfg->bday()) {
		cerr << cfg->loc() << "Object Life Cycle birthday"
			<< "settings are depricated and ignored" << endl;
	}

	// move all the checks from 1.3 here
	cfg->variance(theModVar);
	cfg->withLmt(theShowModRatio);
	theModTime = cfg->length();

	Array<QualifSym*> qs;
	cfg->expires(theExpires, theExpSelector, qs);
	// set rng
	if (theExpSelector)
		theExpSelector->rndGen(&theRng);
	// Rng for other distributions is set before use, because they may
	// be shared between different ObjLifeCicle objects.

	// interpret qualifiers
	for (int i = 0; i < qs.count(); ++i) {
		if (qs[i]) {
			Assert(qs[i]->kind() != QualifSym::qfNone);
			theExpGapTypes.append((ExpGapType)gapType(qs[i]->kind()));
		} else
		if (theExpires[i]) {
			cerr << cfg->loc() << "expire distribution (at position " 
				<< i+1
				<< ") does not have a qualifier (e.g., `lmt' or `now')" 
				<< endl;
			exit(-2);
		} else
			theExpGapTypes.append(egtNone);
	}
}

// calculates modification and expiration times for a given object
// note: result times must be rounded to second resolution or
//       comparison of HTTP-equal dates may not work!
void ObjLifeCycle::calcTimes(int seed, ObjTimes &times) {
	// must have a mod time model to proceed
	if (!theModTime) {
		times.reset();
		return;
	}

	// note: all distributions are linked to this rng
	theRng.seed(seed);
	theModTime->rndGen(&theRng);

	// all time computations are in [sec]

	// used a lot
	const int now = TheClock.time().sec();

	// several "persistent" choices
	const int expIdx = theExpSelector ? (int)theExpSelector->trial() : -1;
	times.doShowLmt = theRng() < theShowModRatio;
	const int cycle = Max(1, (int)theModTime->trial());
	const int birth = theRng(0, cycle);

	// also persistent: expiration gap (used below)
	// the delta is persistent because that's how real servers
	// have it configured: mod + const_delta OR now + const_delta
	RndDistr *expd = expIdx >= 0 ? theExpires[expIdx] : 0;
	if (expd)
		expd->rndGen(&theRng);
	const int expGap = !expd ? 0 : (int)expd->trial();

	// find min cycleCount (lmt without random, cycle dependent delta)
	int cycleCnt = (now - (birth + cycle/2)) / cycle;

	// find LMT and NMT so that NMT is in the future
	// (do not want to produce stale objects)
	int lmt = -1;
	int nmt = birth + lmtDelta(seed, cycleCnt, cycle);
	do {
		lmt = nmt;
		nmt = birth + lmtDelta(seed, ++cycleCnt, cycle);
	} while (nmt <= now);

	times.theLmt = Time::Sec(lmt);

	// calculate expiration time if needed
	if (expd) {
		if (theExpGapTypes[expIdx] == egtNow)
			times.theExp = TheClock + Time::Sec(expGap);
		else
		if (theExpGapTypes[expIdx] == egtNmt)
			times.theExp = Time::Sec(nmt + expGap);
		else
			times.theExp = times.theLmt + Time::Sec(expGap);
	} else
		times.theExp = Time();
}

// compute lmt delta for a given cycle (with a random component)
// note: cycleCnt == number of full (completed) cycles
int ObjLifeCycle::lmtDelta(int seed, int cycleCnt, int cycleLen) {
	if (cycleCnt <= 0)
		return 0;
	theRng.seed(GlbPermut(cycleCnt, seed));
	const double drift = cycleLen/2.0;
	return cycleCnt*cycleLen + (int) (theModVar * theRng(-drift, +drift));
}

int ObjLifeCycle::gapType(int qualifSymKind) const {
	switch (qualifSymKind) {
		case QualifSym::qfLmt:
			return egtLmt;
		case QualifSym::qfNow:
			return egtNow;
		case QualifSym::qfNmt:
			return egtNmt;
		default:
			Assert(false);
	}
	return 0;
}
