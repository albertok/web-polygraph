
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "xstd/Socket.h"
#include "runtime/LogComment.h"
#include "runtime/Notifier.h"
#include "runtime/PolyOLog.h"
#include "runtime/SharedOpts.h"
#include "runtime/StatCycle.h"

#include "xstd/gadgets.h"
#include "runtime/globals.h"
#include "base/polyLogTags.h"
#include "base/polyLogCats.h"


class StatCycleRec: public StatIntvlRec {
};



StatCycle::StatCycle(): theRecs(lgcEnd), theNotifier(0) {
	for (int i = 0; i < theRecs.capacity(); ++i)
		theRecs.push(new StatCycleRec);
}

StatCycle::~StatCycle() {
	while (theRecs.count()) delete theRecs.pop();
}

void StatCycle::period(Time aPeriod) {
	thePeriod = aPeriod;
}

void StatCycle::start() {
	Assert(thePeriod > 0);
	startListen();
	nextIntvl(TheClock + thePeriod);
}

void StatCycle::notifier(Notifier *aNotifier) {
	theNotifier = aNotifier;
}

void StatCycle::wakeUp(const Alarm &alarm) {
	const Time wakeUpTime = Should(alarm.time() > 0) ?
		alarm.time() : TheClock;
	StatIntvl::wakeUp(alarm);
	restart();
	// absolute deadlines to detect overload conditions in best-effort tests
	nextIntvl(wakeUpTime + thePeriod);
}

void StatCycle::nextIntvl(Time wakeUpTime) {
	theIntvlStart = TheClock;
	sleepTill(wakeUpTime);
}

void StatCycle::restart() {
	Assert(theIntvlStart > 0);
	setDuration(theIntvlStart);
	// Do not log stats collected while waiting: There should not be any, and
	// adding empty records will affect naive global averages. This may need to
	// be revisited: We are hiding this [lack of] information from testers.
	if (!TheStatPhaseMgr.trafficWaiting())
		storeAll(TheOLog, lgStatCycleRec);
	if (TheReportCat > 0)
		report(*theRecs[TheReportCat]);
	for (int i = 0; i < theRecs.count(); ++i) {
		StatCycleRec &rec = *theRecs[i];
		if (theNotifier && IsActiveCat[i])
			theNotifier->noteStats(rec, i);
		rec.restart();
	}
}

StatIntvlRec &StatCycle::getRec(int cat) {
	Assert(0 < cat && cat < theRecs.count());
	return *theRecs[cat];
}

const StatIntvlRec &StatCycle::getRec(int cat) const {
	Assert(0 < cat && cat < theRecs.count());
	return *theRecs[cat];
}

void StatCycle::report(const StatCycleRec &rec) const {
	const String phase = TheStatPhaseMgr->name();
	ostream &repAll = Comment(2) << "i-" << phase;
	rec.linePrintAll(repAll, true);
	repAll << endc;

	if (TheOpts.theDumpFlags(dumpEmbedStats)) {
		Comment(2) << "embedded content stats:" << endl
			<< "\ttags seen: " << TheEmbedStats.tagSeen
			<< " matched: " << TheEmbedStats.tagMatched << endl
			<< "\tattrs seen: " << TheEmbedStats.attrSeen
			<< " matched: " << TheEmbedStats.attrMatched << endl
			<< "\tURLs seen: " << TheEmbedStats.urlSeen
			<< endc;
	}

	if (TheEmbedStats.foreignUrlRequested || TheEmbedStats.foreignUrlReceived) {
		ostream &os = Comment(2) << "foreign URLs:"
			<< " requested: " << TheEmbedStats.foreignUrlRequested;
		if (TheEmbedStats.foreignUrlReceived)
			os << " served: " << TheEmbedStats.foreignUrlReceived;
		os << endc;
	}
}
