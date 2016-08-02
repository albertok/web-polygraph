
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_STATPHASE_H
#define POLYGRAPH__RUNTIME_STATPHASE_H

#include "xstd/Array.h"
#include "base/ErrorStat.h"
#include "base/StatPhaseRec.h"
#include "runtime/Goal.h"
#include "runtime/TransFactor.h"
#include "runtime/StatIntvl.h"

class StatPhaseMgr;
class StatsSampleCfg;
class PhaseSym;
class StatsSampleSym;
class Rptmstat;
class DutWatchdog;
class Script;

// stats phase is a named [long] interval during which
// stats are accumulated; the phase goal may not be [just] time based
// the stats are logged when instructed by the manager
// when phase goal is reached stat phase terminates notifying the manager

class StatPhase: public StatIntvl, public GoalSubj {
	public:

		enum LockType {
			ltWssFreeze,  // waiting for WSS freeze
			ltPhaseSync,  // waiting for phase sync
			ltWarmup      // waiting for warmup scan
		};

		// creates a no-logging phase without an explicit PGL configuration
		static StatPhase *MakeVirtual(const String &aName, const StatPhase *prevPh);

		StatPhase();
		virtual ~StatPhase();

		void configure(const PhaseSym *ps, const StatPhase *prevPh);
		void addWatchdog(DutWatchdog *dog);

		const String &name() const { return theName; }
		bool used() const { return wasUsed; }
		bool unlockToStop() const { return readyToStop; }
		bool mustStop() const { return reachedNegative; }
		const GoalRec &goal() const { return theGoal; }
		const TransFactor &populusFactor() const { return thePopulusFactor; }
		const TransFactor &loadFactor() const { return theLoadFactor; }
		const TransFactor &recurFactor() const { return theRecurFactor; }
		const TransFactor &specialMsgFactor() const { return theSpecialMsgFactor; }

		// finalize GoalSubj interface
		virtual Time duration() const;
		virtual Counter xactCnt() const;
		virtual BigSize fillSz() const;
		virtual Counter fillCnt() const;
		virtual Counter xactErrCnt() const;

		TransFactor &populusFactor() { return thePopulusFactor; }
		TransFactor &loadFactor() { return theLoadFactor; }
		OidGenStat &oidGenStat();
		ErrorStat &errors(int logCat);
		RangeGenStat &rangeGenStat();

		void statsLogged(bool are);
		void name(const String &aName);

		void lock(const LockType lt);
		void unlock(const LockType lt);

		void start(StatPhaseMgr *aMgr, const StatPhase *prevPhase);
		void stop();
		void flush();

		void reachedPositiveGoal(const String &reason);

		virtual void wakeUp(const Alarm &alarm);

		virtual void noteConnEvent(BcastChannel *ch, const Connection *c);
		virtual void noteXactEvent(BcastChannel *ch, const Xaction *x);
		virtual void noteCompoundXactEvent(BcastChannel *ch, const CompoundXactInfo *cx);
		virtual void noteIcpXactEvent(BcastChannel *ch, const IcpXaction *x);
		virtual void notePageEvent(BcastChannel *ch, const PageInfo *p);
		virtual void noteInfoEvent(BcastChannel *ch, InfoEvent ev);

		// should these be replaced with BcastChannels
		void noteSockRead(Size sz, int cat) { theRecs[cat]->theSockRdSzH.record(sz); }
		void noteSockWrite(Size sz, int cat) { theRecs[cat]->theSockWrSzH.record(sz); }

		virtual StatIntvlRec &getRec(int cat);
		virtual const StatIntvlRec &getRec(int cat) const;

		void reportCfg(ostream &os) const;

	protected:
		void configureSamples(const PhaseSym *cfg);
		void finalizeStats();

		virtual bool checkpoint();

		bool locked(const int lt) const;
		bool locked() const;
		void report() const;
		void printGoalReached(const bool positive, const String &reason = "") const;

	protected:
		Array<StatPhaseRec*> theRecs;
		Array<StatsSampleCfg*> theSamples;

		StatPhaseMgr *theMgr;
		String theName;
		Goal theGoal;          // configurable GoalRec with reached()
		Rptmstat *theRptmstat;
		Script *theScript;
		Array<DutWatchdog*> theDutWatchdogs;

		TransFactor thePopulusFactor;
		TransFactor theLoadFactor;
		TransFactor theRecurFactor;
		TransFactor theSpecialMsgFactor;

		Array<int> theLocks; // lock level for each lock type

		bool wasUsed;          // was listening for events
		bool logStats;         // log stats when flushed
		bool waitWssFreeze;    // wait for working sset to freeze
		bool doSynchronize;    // sync with remote phases
		bool readyToStop;      // locked after the was met
		bool wasStopped;       // stop() was called
		bool primary;          // true if the phase is primary
		bool reachedPositive;  // true if positive goal is reached
		bool reachedNegative;  // true if negative goal is reached
};


#endif
