
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/CompoundXactInfo.h"


CompoundXactInfo *CompoundXactInfo::TheFree = 0;


CompoundXactInfo *CompoundXactInfo::Create(const int logCat) {
	CompoundXactInfo *info = 0;
	if (TheFree) {
		info = TheFree;
		TheFree = TheFree->theNext;
		info->reset();
	} else
		info = new CompoundXactInfo();

	info->logCat = logCat;
	++info->ccLevel;
	return info;
}

void CompoundXactInfo::Abandon(CompoundXactInfo *&info) {
	if (!info)
		return;

	if (ShouldUs(info->ccLevel > 0))
		--info->ccLevel;

	if (info->ccLevel <= 0) {
		Should(!info->theNext);
		info->theNext = TheFree;
		TheFree = info;
	}
	// else others will still hold on to the shared info

	info = 0;
}

// XXX: Should set *Xact::theCompound, but only the caller can access that.
void CompoundXactInfo::Share(CompoundXactInfo *info) {
	ShouldUs(!info->completed());
	++info->ccLevel;
}

CompoundXactInfo::CompoundXactInfo() {
	reset();
}

CompoundXactInfo::~CompoundXactInfo() {
}

void CompoundXactInfo::reset() {
	logCat = 0;
	ccLevel = 0;

	reqSize = repSize = 0;
	startTime = finishTime = Time();
	exchanges = 0;

	connectState = opNone;
	proxyAuthState = opNone;
	proxyStatAuth = AuthPhaseStat::sNone;

	theNext = 0;
}

void CompoundXactInfo::record(CompoundXactStat &s) const {
	if (!(ShouldUs(finishTime > 0) && ShouldUs(startTime > 0)))
		return;

	const Time rptm = ShouldUs(finishTime >= startTime) ?
		finishTime - startTime : Time(0,0);

	s.recordCompound(rptm, reqSize, repSize, exchanges);
}
