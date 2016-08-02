
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/math.h"
#include "xstd/h/iomanip.h"
#include "xstd/gadgets.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "base/Progress.h"
#include "base/ProtoIntvlStat.h"

ProtoIntvlStat::ProtoIntvlStat():
	updateProgress(false),
	theProgress(0),
	theErrXacts(0) {
}

void ProtoIntvlStat::progress(ProtoProgress *aProgress) {
	Assert(!theProgress && aProgress);
	theProgress = aProgress;
}

const char *ProtoIntvlStat::name() const {
	Assert(theProgress);
	return theProgress->name;
}

const char *ProtoIntvlStat::id() const {
	Assert(theProgress);
	return theProgress->id;
}

void ProtoIntvlStat::restart() {
	theDoneXacts.reset();
	theXactLvl.restart();
	theConnLvl.restart();
	theErrXacts = 0;
}

bool ProtoIntvlStat::active() const {
	return theDoneXacts.active() || theXactLvl.active() ||
		theConnLvl.active() || theErrXacts > 0;
}

bool ProtoIntvlStat::sane() const {
	return theDoneXacts.sane() && theXactLvl.sane() &&
		theConnLvl.sane() && theErrXacts >= 0;
}

void ProtoIntvlStat::recordXact(const Time &rptm, const Size &sz, const bool hit) {
	// Hack: We are called for both StatCycle and StatPhase stats but
	// theProject is global and only one ProtoIntvlStat should update it.
	if (updateProgress) {
		Assert(theProgress);
		++theProgress->successes;
	}
	theDoneXacts.record(rptm, sz, hit);
}

void ProtoIntvlStat::keepLevel(const ProtoIntvlStat &s) {
	theXactLvl.keepLevel(s.theXactLvl);
	theConnLvl.keepLevel(s.theConnLvl);
}

// keep this and others in sync with StatIntvlRec::reqRate and others
double ProtoIntvlStat::reqRate(Time duration) const {
	return duration > 0 ?
		Ratio(theXactLvl.incCnt(), duration.secd()) : -1;
}

double ProtoIntvlStat::repRate(Time duration) const {
	return duration > 0 ?
		Ratio(theXactLvl.decCnt(), duration.secd()) : -1;
}

double ProtoIntvlStat::reqBwidth(Time duration) const {
	return duration > 0 ?
		theDoneXacts.xacts().size().mean() * reqRate(duration) : -1;
}

double ProtoIntvlStat::repBwidth(Time duration) const {
	return duration > 0 ?
		Ratio(theDoneXacts.xacts().size().sum(), duration.secd()) : -1;
}

OLog &ProtoIntvlStat::store(OLog &log) const {
	const bool a = active();
	log << a;
	if (a)
		log << theDoneXacts << theXactLvl << theConnLvl << theErrXacts;
	return log;	
}

ILog &ProtoIntvlStat::load(ILog &log) {
	if (log.getb()) // active?
		log >> theDoneXacts >> theXactLvl >> theConnLvl >> theErrXacts;
	else
		restart();
	return log;
}

void ProtoIntvlStat::merge(const ProtoIntvlStat &s) {
	theXactLvl.merge(s.theXactLvl);
	theConnLvl.merge(s.theConnLvl);
	join(s);
}

void ProtoIntvlStat::concat(const ProtoIntvlStat &s) {
	theXactLvl.concat(s.theXactLvl);
	theConnLvl.concat(s.theConnLvl);
	join(s);
}

void ProtoIntvlStat::join(const ProtoIntvlStat &s) {
	theDoneXacts += s.theDoneXacts;
	theErrXacts += s.theErrXacts;
}

ostream &ProtoIntvlStat::print(ostream &os, const String &pfx, Time duration) const {
	os << pfx << "req.rate:\t " << reqRate(duration) << endl;
	os << pfx << "rep.rate:\t " << repRate(duration) << endl;

	theDoneXacts.xacts().print(os, pfx + "rep.");
	theDoneXacts.print(os, "hit", "miss", pfx);
	theXactLvl.print(os, pfx + "xact.");
	theConnLvl.print(os, pfx + "conn.");

	os << pfx << "errors.count:\t " << theErrXacts << endl;
	return os;
}

void ProtoIntvlStat::linePrint(ostream &os, int offset, Time duration, bool includeLevels) const {
	Assert(theProgress);

	if (!active())
		return;

	os << endl
		<< setw(offset) << theProgress->id
		<< ' ' << setw(6) << theProgress->successes
		<< ' ' << setw(6) << repRate(duration)
		<< ' ' << setw(6) << (int)rint(doneXacts().xacts().time().mean())
		<< ' ' << setw(6) << doneXacts().dhp()
		<< ' ' << setw(3) << errXacts()
		;

	// XXX: we are not counting or reporting proto-related listen sockets
	if (includeLevels)
		os << ' ' << setw(4) << connLevel().level();
}
