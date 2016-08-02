
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Clock.h"
#include "xstd/gadgets.h"
#include "runtime/ObjUniverse.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/StatPhase.h"
#include "runtime/Viservs.h"
#include "runtime/HostMap.h"
#include "runtime/LogComment.h"
#include "client/ServerRep.h"
#include "client/WarmupPlan.h"

int WarmupPlan::TheReqCnt = 0;


WarmupPlan::WarmupPlan(const Array<int> &visIdx): thePlan(visIdx.count()) {
	for (int i = 0; i < visIdx.count(); ++i)
		thePlan.enqueue(visIdx[i]);
	static RndGen rng;
	thePlan.randomize(rng);
	TheStatPhaseMgr->lock(StatPhase::ltWarmup);
}

WarmupPlan::~WarmupPlan() {
	// if Polygraph shutdowns before the warmup is complete, we
	// may get here after the phases are deleted
	if (*TheStatPhaseMgr)
		TheStatPhaseMgr->unlock(StatPhase::ltWarmup);
}

int WarmupPlan::selectViserv() {
	int viserv = -1;

	while (!thePlan.empty() && viserv < 0) {
		const int vs = thePlan.dequeue();
		if (WarmViserv(vs))
			continue;
		thePlan.enqueue(vs);
		viserv = vs;
		TheReqCnt++;
	}

	if (thePlan.count())
		ReportProgress();

	return viserv;
}

// warm if any of the targets was successfully accessed
// or at least two access attempts have been made
bool WarmupPlan::WarmViserv(int viserv) {
	const HostCfg *hcfg = TheHostMap->at(viserv);
	if (Should(hcfg && hcfg->theServerRep)) {
		return hcfg->theServerRep->responses() > 0 ||
			hcfg->theServerRep->requests() > 1;
	}
	return false;
}

void WarmupPlan::ReportProgress() {
	const Time reportGap = Time::Sec(45); // how often to report
	static const Time start = TheClock;
	static Time nextReport = start + reportGap;

	if (nextReport >= TheClock)
		return;

	const int visibleCnt = ObjUniverse::Count();
	const int readyCnt = ReadyCount();
	const double readyPct = Percent(readyCnt, visibleCnt);

	Comment(5) << "fyi:"
		<< " servers visible: " << visibleCnt
		<< " warm: " << readyCnt << " (" << readyPct << "%)"
		<< " attempts: " << TheReqCnt
		<< endc;

	nextReport = TheClock + reportGap;
}

int WarmupPlan::ReadyCount() {
	int count = 0;
	for (ViservIterator i; !i.atEnd(); ++i) {
		if (WarmViserv(i.viserv()))
			count++;
	}
	return count;
}

