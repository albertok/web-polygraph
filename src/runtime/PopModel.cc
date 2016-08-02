
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/math.h"

#include "xstd/Assert.h"
#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "csm/ContentMgr.h"
#include "csm/ContentCfg.h"
#include "csm/ContentSel.h"
#include "pgl/PopDistr.h"
#include "pgl/PopModelSym.h"
#include "runtime/HostMap.h"
#include "runtime/PopModel.h"


PopModel::PopModel(): theDistr(0), theHotSetProb(-1), theHotSetFrac(1), theBhrDiscr(0) {
}

void PopModel::configure(const PopModelSym *cfg) {
	theDistr = cfg->popDistr();
	cfg->hotSetFrac(theHotSetFrac);
	cfg->hotSetProb(theHotSetProb);
	cfg->bhrDiscr(theBhrDiscr);

	if (!theDistr) {
		cerr << cfg->loc() << "popularity distribution is required if PopModel is used" << endl;
		exit(-2);
	}
}

void PopModel::choose(Counter lastOid, Counter wss, Counter hotSetPos, ObjId &oid) {
	Assert(wss != 0);
	Assert(lastOid > 0);

	static RndGen rng;
	const bool hot = theHotSetProb > 0 && rng.event(theHotSetProb);
	oid.hot(hot);

	// adjust params if we need a "hot" object
	if (hot) {
		wss = wss > 0 ? Max(1., rint(wss*theHotSetFrac)) : lastOid;
		lastOid = MiniMax(wss, hotSetPos, lastOid);
	}

	const Counter offset = (0 < wss && wss < lastOid) ? lastOid-wss : 0;
	const Counter oname = offset + theDistr->choose(rng, lastOid - offset);
	Assert(0 < oname && oname <= lastOid);

	const int searchSwing = rng.event(theBhrDiscr) ? 4 : 0;
	const Counter nameBeg =
		Max(static_cast<Counter>(1), oname - searchSwing);
	const Counter nameEnd = Min(oname + searchSwing, lastOid) + 1;
	pickBest(nameBeg, nameEnd, oid);
}

// pick best from nameBeg to nameEnd, excluding nameEnd
void PopModel::pickBest(const Counter nameBeg, const Counter nameEnd, ObjId &oid) {
	const Counter defName = (nameBeg + nameEnd - 1) / 2;
	oid.name(defName);

	// foreign objects have unknown size and cachability status
	if (oid.foreignUrl()) 
		return;

	const HostCfg *hcfg = TheHostMap->at(oid.target());
	Assert(hcfg);
	Assert(hcfg->theContent);

	// apply size discrimination to "cachable" groups only
	Assert(oid.type() >= 0);
	const ContentCfg &ccfg = *TheContentMgr.get(oid.type());
	if (!ccfg.calcCachability(oid))
		return;

	// find oid with the smallest response size
	Size bestSize;
	Counter bestName = -1;
	for (Counter name = nameBeg; name < nameEnd; ++name) {
		oid.name(name);
		if (!ccfg.calcCachability(oid))
			continue;

		const Size sz = ccfg.calcRawRepSize(oid);
		if (bestName < 0 || sz < bestSize) {
			bestName = name;
			bestSize = sz;
		}
	}

	Assert(bestName > 0);
	oid.name(bestName);
}

#if 0
// pick best from nameBeg to nameEnd, excluding nameEnd
void PopModel::pickBest(const Counter nameBeg, const Counter nameEnd, ObjId &oid) {
	const HostCfg *hcfg = TheHostMap->at(oid.target());
	Assert(hcfg);
	Assert(hcfg->theContent);

	// apply discrimination to "cachable" groups only
	const Counter defName = (nameBeg + nameEnd - 1) / 2;
	oid.name(defName);
	Assert(oid.type() >= 0);
	const ContentCfg &ccfg = *TheContentMgr.get(oid.type());
	if (!ccfg.calcCachability(oid))
		return;

	// extract probabilities and calculate the sum
	static Array<double> probs;
	probs.reset();
	probs.stretch(nameEnd-nameBeg);
	double sum = 0;
	{for (Counter name = nameBeg; name < nameEnd; ++name) {
		oid.name(name);
		const double p = ccfg.calcCachability(oid) ?
			ccfg.recurrence() : 0.0;
		probs.append(p);
		sum += p;
	}}

	// find name based on collected probabilities
	static RndGen rng;
	const double mark = sum*rng.trial(); // multiply once to avoid div in loop
	double pos = 0;
	{for (Counter i = 0, name = nameBeg; name < nameEnd; ++i, ++name) {
		const double p = probs[i];
		if (pos <= mark && mark < pos + p) {
			oid.name(name);
			return;
		}
		pos += p;
	}}

	// default (should not happen except due to rounding effects)
	oid.name(nameBeg);
}
#endif

