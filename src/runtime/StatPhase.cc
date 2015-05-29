
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/math.h"
#include "xstd/h/iomanip.h"

#include "xstd/Clock.h"
#include "base/Progress.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "pgl/PglCodeSym.h"
#include "pgl/StatsSampleCfg.h"
#include "pgl/StatsSampleSym.h"
#include "pgl/GoalSym.h"
#include "pgl/PhaseSym.h"
#include "runtime/Connection.h"
#include "runtime/Goal.h"
#include "runtime/HttpCookies.h"
#include "runtime/httpHdrs.h"
#include "runtime/Xaction.h"
#include "runtime/IcpXaction.h"
#include "runtime/LogComment.h"
#include "runtime/PageInfo.h"
#include "runtime/CompoundXactInfo.h"
#include "runtime/SharedOpts.h"
#include "runtime/PolyOLog.h"
#include "runtime/Rptmstat.h"
#include "runtime/Script.h"
#include "runtime/DutWatchdog.h"
#include "runtime/StatsSampleMgr.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/StatPhaseSync.h"
#include "runtime/StatPhase.h"
#include "runtime/globals.h"
#include "runtime/polyErrors.h"
#include "runtime/polyBcastChannels.h"

class CondCallSym;

// XXX: lots of client-side specific logic; split

static enum { wssIgnore, wssWaitFreeze, wssFrozen } TheWssState = wssIgnore;

static const String TheLockNames[] = { "WSS freeze", "phase sync", "warmup" };
const int NumberOfLocks = sizeof(TheLockNames)/sizeof(*TheLockNames);

StatPhase::StatPhase(): theRecs(lgcEnd), theMgr(0), 
	theRptmstat(0), theScript(0),
	thePopulusFactor(this, "populus"), theLoadFactor(this, "load"), 
	theRecurFactor(this, "recurrence"), 
	theSpecialMsgFactor(this, "special message"),
	theLocks(NumberOfLocks),
	wasUsed(false), logStats(true), waitWssFreeze(false), doSynchronize(true),
	readyToStop(false), wasStopped(false), primary(false),
	reachedPositive(false), reachedNegative(false) {

	for (int i = 0; i < theRecs.capacity(); ++i)
		theRecs.push(new StatPhaseRec);

	for (int i = 0; i < theLocks.capacity(); ++i)
		theLocks.push(0);

	theChannels << TheInfoChannel;
}

StatPhase::~StatPhase() {
	delete theRptmstat; theRptmstat = 0;
	delete theScript; theScript = 0;
	while (theRecs.count()) delete theRecs.pop();
	while (theDutWatchdogs.count()) delete theDutWatchdogs.pop();
}

void StatPhase::configure(const PhaseSym *cfg, const StatPhase *prevPh) {
	Assert(cfg);
	Assert(cfg->goal());

	name(cfg->name());
	theGoal.configure(*cfg->goal());
	for (int i = 0; i < theRecs.count(); ++i)
		theRecs[i]->theGoal = theGoal;

	cfg->logStats(logStats);
	cfg->waitWssFreeze(waitWssFreeze);
	cfg->synchronize(doSynchronize); // true by default

	cfg->primary(primary);
	if (primary) {
		for (int i = 0; i < theRecs.count(); ++i)
			theRecs[i]->primary = primary;
	}

	double b = -1;
	double e = -1;
	cfg->populusFactorBeg(b);
	cfg->populusFactorEnd(e);
	thePopulusFactor.configure(b, e, prevPh ? &prevPh->thePopulusFactor : 0);
	checkFactor(thePopulusFactor, "populus");

	b = -1;
	e = -1;
	cfg->loadFactorBeg(b);
	cfg->loadFactorEnd(e);
	theLoadFactor.configure(b, e, prevPh ? &prevPh->theLoadFactor : 0);
	checkFactor(theLoadFactor, "load");

	b = -1;
	e = -1;
	cfg->recurFactorBeg(b);
	cfg->recurFactorEnd(e);
	theRecurFactor.configure(b, e, prevPh ? &prevPh->theRecurFactor : 0);
	checkFactor(theRecurFactor, "recurrence");

	b = -1;
	e = -1;
	cfg->specialMsgFactorBeg(b);
	cfg->specialMsgFactorEnd(e);
	theSpecialMsgFactor.configure(b, e, prevPh ? &prevPh->theSpecialMsgFactor : 0);
	checkFactor(theSpecialMsgFactor, "special_req");

	if (RptmstatSym *s = cfg->rptmstat()) {
		theRptmstat = new Rptmstat;
		theRptmstat->configure(*s);
	}

	if (const CodeSym *code = cfg->script())
		theScript = new Script(*code);

	configureSamples(cfg);
}

void StatPhase::configureSamples(const PhaseSym *cfg) {
	Array<const StatsSampleSym*> ssyms;
	if (cfg->statsSamples(ssyms)) {
		for (int i = 0; i < ssyms.count(); ++i) {
			const StatsSampleSym &ssym = *ssyms[i];

			StatsSampleCfg cfg;
			// XXX: append counter to phase names
			cfg.name = ssym.name() ? ssym.name() : name();
			Assert(ssym.capacity(cfg.capacity));
			cfg.start = ssym.start();

			if (cfg.start >= 0) {
				TheStatsSampleMgr.addSample(cfg);
			} else {
				theSamples.append(new StatsSampleCfg(cfg));
				TheStatsSampleMgr.willAddSample();
			}
		}
	}
	// no support for multiple pending samples for now
	Assert(theSamples.count() <= 1); 
}

void StatPhase::addWatchdog(DutWatchdog *dog) {
	theDutWatchdogs.append(dog);
}

void StatPhase::checkFactor(const TransFactor &f, const String &label) const {
	if (f.beg() > 1 || f.end() > 1) {
		cerr << "error: begin or end value of " << label <<
			" factor for phase '" << theName <<
			"' is greater than 100%" << endl << xexit;
	}

	// varying load is possible only if goal is specified
	const bool varyF = f.beg() != f.end();
	if (!theGoal && varyF) {
		cerr << "phase named `" << theName
			<< "' has " << label
			<< " factors but has no positive goal" << endl;
		exit(-2);
	} else
	if (f.flat() && varyF) {
		cerr << "internal error: no change coefficient for " << label
			<< " factors for the phase named '" << theName << "'" << endl;
		exit(-2);
	}
}

void StatPhase::finalizeStats() {
	setDuration(theIntvlStart);

	for (int i = 0; i < IsActiveCat.count(); ++i) {
		if (IsActiveCat[i]) {
			StatPhaseRec &rec = *theRecs[i];
			rec.theCookiesPurgedFresh += HttpCookies::ThePurgedFreshCount;
			rec.theCookiesPurgedStale += HttpCookies::ThePurgedStaleCount;
			rec.theCookiesUpdated += HttpCookies::TheUpdatedCount;
		}
	}
	HttpCookies::ThePurgedFreshCount = 0;
	HttpCookies::ThePurgedStaleCount = 0;
	HttpCookies::TheUpdatedCount = 0;
}

void StatPhase::lock(const LockType lt) {
	++theLocks[lt];
	Comment(10) << "fyi: current lock level for '"<< TheLockNames[lt]
		<< "' increased to " << theLocks[lt] << endc;
}

void StatPhase::unlock(const LockType lt) {
	if (locked(lt)) {
		--theLocks[lt];
		Comment(10) << "fyi: current lock level for '"<< TheLockNames[lt]
			<< "' decreased to " << theLocks[lt] << endc;
		if (!locked())
			checkpoint();
	} else {
		Comment(0)
			<< "internal error: attempt to unlock an unlocked lock '"
			<< TheLockNames[lt]
			<< "' (salvaged)" << endc;
	}
}

bool StatPhase::locked(const int lt) const {
	Assert(0 <= lt && lt <= theLocks.count());
	return theLocks[lt];
}

bool StatPhase::locked() const {
	// TODO: optimize by maintaining the 'locked' state
	for (int i = 0; i < theLocks.count(); ++i)
		if (locked(i))
			return true;
	return false;
}

Time StatPhase::duration() const {
	return TheClock.time() >= theIntvlStart ?
		TheClock - theIntvlStart : Time(0,0);
}

int StatPhase::xactCnt() const {
	int count = 0;
	for (int i = 0; i < theRecs.count(); ++i)
		count += theRecs[i]->xactCnt();
	return count;
}

BigSize StatPhase::fillSz() const {
	BigSize fill(0);
	for (int i = 0; i < theRecs.count(); ++i)
		fill += theRecs[i]->totFillSize();
	return fill;
}

int StatPhase::fillCnt() const {
	int fill = 0;
	for (int i = 0; i < theRecs.count(); ++i)
		fill += theRecs[i]->totFillCount();
	return fill;
}

int StatPhase::xactErrCnt() const {
	int count = 0;
	for (int i = 0; i < theRecs.count(); ++i)
		count += theRecs[i]->theXactErrCnt;
	return count;
}

void StatPhase::start(StatPhaseMgr *aMgr, const StatPhase *prevPhase) {
	Assert(!theMgr);
	theMgr = aMgr;

	if (prevPhase) {
		for (int i = 0; i < theRecs.count(); ++i)
			theRecs[i]->keepLevels(*prevPhase->theRecs[i]);
	}

	// XXX: will not lock if fill phase finishes before we get ieWssFill
	if (waitWssFreeze && TheWssState == wssWaitFreeze)
		lock(ltWssFreeze);

	if (theRptmstat)
		theRptmstat->start(this);

	if (theScript)
		theScript->run();

	TheOLog << bege(lgStatPhaseBeg, lgcAll) << name() << ende;

	startListen();
	theIntvlStart = TheClock;

	for (int i = 0; i < theSamples.count(); ++i) {
		StatsSampleCfg &cfg = *theSamples[i];
		if (cfg.start < 0) {
			cfg.start = theIntvlStart;
			if (theGoal.duration() > 0)
				cfg.start += theGoal.duration()/2; // middle of the phase
			cfg.start -= Clock::TheStartTime;
		}
		TheStatsSampleMgr.addSample(cfg);
	}

	// manager should be careful with immediate "returns"
	if (!checkpoint()) { 
		// not an immediate exit
		wasUsed = true; 
		if (theGoal.duration() > 0)
			sleepFor(theGoal.duration());
	}
}

void StatPhase::noteConnEvent(BcastChannel *ch, const Connection *c) {
	if (ch == TheConnCloseChannel && !c->bad()) {
		StatPhaseRec &rec = *theRecs[c->logCat()];

		const int depth = c->useLevelMax();
		if (depth > 1)
			rec.theConnPipelineDepths.record(depth);

		const Time ttl = TheClock - c->openTime();
		rec.theConnClose.record(c->closeKind(), ttl, c->useCnt());
	}
	StatIntvl::noteConnEvent(ch, c);
}

void StatPhase::noteXactEvent(BcastChannel *ch, const Xaction *x) {
	StatPhaseRec &rec = *theRecs[x->logCat()];
	const Time repTime = x->lifeTime();
	const Size repSize = x->repSize().actual();

	if (ch == TheXactEndChannel) {
		const ObjId &oid = x->oid();
		const Size reqSize = x->reqSize().actual();
		const bool authing = x->authing();

		// stats must be recorded in only one category for totals to work!
		if (oid.aborted())
			; // do nothing, StatIntvl has enough(?) stats for these
		else
		if (authing) {
			Assert(x->logCat() == lgcCltSide);
			// do not record anything for authing transactions
		} else
		if (oid.basic()) {
			rec.theBasicXacts.record(repTime, repSize, oid.hit());
			rec.recordByteTime(x->logCat(), x->lastReqByteTime(), x->firstRespByteTime());
		} else
		if (oid.active() || oid.passive())
			rec.theFtpXacts.record(repTime, repSize, oid.active());
		else
		if (oid.connect())
			rec.theConnectXacts.record(repTime, repSize);
		else
		if (oid.repToRedir())
			rec.theRepToRedirXacts.record(repTime, repSize);
		else
		if (oid.rediredReq())
			rec.theRediredReqXacts.record(repTime, repSize);
		else
		if (oid.imsAny()) {
			rec.theImsXacts.record(repTime, repSize, x->httpStatus() == RepHdr::sc200_OK);
			if (x->logCat() == lgcSrvSide &&
				x->reqFlags() >= 0 &&
				!(x->reqFlags() & Xaction::xfValidation))
				rec.theProxyValidationR.record(repTime, repSize, x->httpStatus() == RepHdr::sc200_OK);
		} else
		if (oid.reload())
			rec.theReloadXacts.record(repTime, repSize);
		else
		if (oid.range())
			rec.theRangeXacts.record(repTime, repSize);
		else
		if (oid.head())
			rec.theHeadXacts.record(repTime, repSize);
		else
		if (oid.post())
			rec.thePostXacts.record(repTime, repSize);
		else
		if (oid.put())
			rec.thePutXacts.record(repTime, repSize);
		else {
			Should(false); // all categories should be accounted for
		}

		rec.theRepContType.record(x->actualRepType(), repSize);
		// XXX: request content type is not determined on server side
		rec.theReqContType.record(x->reqOid().type(), reqSize);

		// compound stats
		if (const CompoundXactInfo *const compound = x->partOf()) {
			if (compound->final)
				switch (x->proxyAuth()) {
					case authNone: {
						// XXX: we should not be here, but it happens
						static unsigned int errors = 0;
						Should(errors % 100000 != 0);
						++errors;
						compound->record(rec.theIsolated);
						break;
					}
					case authBasic:
						compound->record(rec.theAuthBasic);
						break;
					case authNtlm:
						compound->record(rec.theAuthNtlm);
						break;
					case authNegotiate:
						compound->record(rec.theAuthNegotiate);
						break;
					default:
						Should(false);
						compound->record(rec.theIsolated);
						break;
				}
		} else {
			static CompoundXactInfo &compound = *CompoundXactInfo::Create();
			compound.exchanges = 1;
			compound.reqSize = reqSize;
			compound.repSize = repSize;
			compound.lifeTime = repTime;
			compound.record(rec.theIsolated);
		}

		if (x->cookiesSent() > 0)
			rec.theCookiesSent.record(x->cookiesSent());
		if (x->cookiesRecv() > 0)
			rec.theCookiesRecv.record(x->cookiesRecv());
	}

	if (ch == TheXactEndChannel ||
		ch == TheXactRetrChannel ||
		ch == TheXactErrChannel) {
		rec.theStatusCode.record(x->httpStatus(), repTime, repSize);
		if (x->inCustomStatsScope())
			rec.theCustomXacts.record(repTime, repSize);
	}

	StatIntvl::noteXactEvent(ch, x);
}

void StatPhase::noteIcpXactEvent(BcastChannel *ch, const IcpXaction *x) {
	if (ch == TheIcpXactEndChannel) {
		StatPhaseRec &rec = *theRecs[x->logCat()];
		if (!x->timedout())
			rec.theIcpXacts.record(x->lifeTime(), x->repSize(), x->hit());
	} else {
		Assert(false);
	}

	StatIntvl::noteIcpXactEvent(ch, x);
}

void StatPhase::notePageEvent(BcastChannel *ch, const PageInfo *p) {
	if (ch == ThePageEndChannel) {
		StatPhaseRec &rec = *theRecs[lgcCltSide];
		rec.thePageHist.record(p->lifeTime, p->size);
	} else {
		Assert(false);
	}
	StatIntvl::notePageEvent(ch, p);
}

void StatPhase::noteInfoEvent(BcastChannel *ch, InfoEvent ev) {
	Assert(ch == TheInfoChannel);
	if (ev == ieWssFill) {
		if (waitWssFreeze)
			lock(ltWssFreeze);
		TheWssState = wssWaitFreeze;
	} else
	if (ev == ieWssFreeze) {
		if (waitWssFreeze)
			unlock(ltWssFreeze);
		TheWssState = wssFrozen;
	} else
	if (ev == ieReportProgress) {
		Comment(7) << "fyi: phase progress:" << endl;

		theGoal.reportProgress(Comment, *this);

		if (TheWssState == wssWaitFreeze) {
			if (waitWssFreeze)
				Comment << "\twaiting for WSS to freeze" << endl;
		} else
		if (TheWssState == wssFrozen)
			Comment << "\tWSS frozen" << endl;

 		Comment << endc;
	}
}

void StatPhase::name(const String &aName) {
	theName = aName;
	if (!theName)
		theName = "";
	for (int i = 0; i < theRecs.count(); ++i)
		theRecs[i]->theName = theName;
}

OidGenStat &StatPhase::oidGenStat() {
	return theRecs[lgcCltSide]->theOidGen;
}

ErrorStat &StatPhase::errors(int logCat) {
	return theRecs[logCat]->theErrors;
}

RangeGenStat &StatPhase::rangeGenStat() {
	return theRecs[lgcCltSide]->theRangeGen;
}

void StatPhase::statsLogged(bool are) {
	logStats = are;
}

void StatPhase::wakeUp(const Alarm &alarm) {
	StatIntvl::wakeUp(alarm);
	// checkpoint() may lock the phase
	Assert(locked() || checkpoint() || locked());
}

void StatPhase::flush() {
	stop();

	if (logStats)
		storeAll(TheOLog, lgStatPhaseRec);
}

// the caller should not use the phase if checkpoint returns true
bool StatPhase::checkpoint()  {
	if (!reachedPositive) {
		reachedPositive = theGoal.reachedPositive(*this);
		if (reachedPositive)
			printGoalReached(true);
	}
	if (!reachedNegative) {
		reachedNegative = theGoal.reachedNegative(*this);
		if (reachedNegative)
			printGoalReached(false);
	}
	const bool reachedEnd = (!locked() && reachedPositive) || reachedNegative;
	if (reachedEnd) {
		if (!reachedNegative && doSynchronize) {
			if (!readyToStop) {
				readyToStop = true; // must be before waitGroupCount()

				Comment(5) << "fyi: local phase `" << name()
					<< "' reached synchronization point" << endc;

				if (int rCount = TheStatPhaseSync.waitGroupCount()) {
					Comment(10) << "waiting for " << rCount 
						<< " more remote sync messages" << endc;
					lock(ltPhaseSync); // somebody needs to unlock us to move on
					return false;
				}
			}
		}

		stop();
		report();

		Assert(theMgr);
		StatPhaseMgr *mgr = theMgr;
		theMgr = 0;
		mgr->noteDone(this);
		return true;
	}
	return false;
}

void StatPhase::stop() {
	Assert(theIntvlStart >= 0);
	if (wasStopped) // already stopped
		return;
	wasStopped = true;

	if (theRptmstat)
		theRptmstat->stop(this);

	for (int d = 0; d < theDutWatchdogs.count(); ++d)
		theDutWatchdogs[d]->stop();

	finalizeStats();
	stopListen();
	TheAlarmClock.cancelAll(this);

	TheOLog << bege(lgStatPhaseEnd, lgcAll) << name() << ende;
}

StatIntvlRec &StatPhase::getRec(int cat) {
	Assert(0 < cat && cat < theRecs.count());
	StatIntvlRec &stats = *theRecs[cat];
	stats.theDuration = duration();
	return stats;
}

const StatIntvlRec &StatPhase::getRec(int cat) const {
	Assert(0 < cat && cat < theRecs.count());
	return *theRecs[cat];
}

// XXX: we probably should report _all_ active sides
void StatPhase::report() const {
	ostream &repAll = Comment(1) << "p-" << name();
	if (TheReportCat > 0)
		theRecs[TheReportCat]->linePrintAll(repAll, false);
	repAll << endc;
}

void StatPhase::reportCfg(ostream &os) const {
	os 
		<< setw(10) << name()
		<< ' ' << setw(8) << thePopulusFactor.beg()
		<< ' ' << setw(8) << thePopulusFactor.end()
		<< ' ' << setw(8) << theLoadFactor.beg()
		<< ' ' << setw(8) << theLoadFactor.end()
		<< ' ' << setw(8) << theRecurFactor.beg()
		<< ' ' << setw(8) << theRecurFactor.end()
		<< ' ' << setw(8) << theSpecialMsgFactor.beg()
		<< ' ' << setw(8) << theSpecialMsgFactor.end()
		<< ' ' << setw(5) << theGoal
		<< "\t" 
			<< (logStats ? "" : " !log")
			<< (waitWssFreeze ? " wwss" : "")
			<< (doSynchronize ? "" : " !sync")
			<< (primary ? " prime" : "")
		<< endl;
}

void StatPhase::printGoalReached(const bool positive) const {
	ostream &os = Comment(1);
	if (locked()) {
		os << "locked (";
		bool printed = false;
		for (int i = 0; i < theLocks.count(); ++i) {
			if (locked(i)) {
				if (printed)
					os << ", ";
				else
					printed = true;
				os << TheLockNames[i];
			}
		}
		os << ") ";
	}
	os
		<< "phase '" << name() << "' reached local "
		<< (positive ? "positive" : "negative")
		<< " goal"
		<< endc;
}
