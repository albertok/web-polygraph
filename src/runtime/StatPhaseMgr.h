
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_STATPHASEMGR_H
#define POLYGRAPH__RUNTIME_STATPHASEMGR_H

#include "xstd/Array.h"
#include "runtime/StatIntvl.h"

class StatPhase;

// iterates through stat phases
// flushes all sats and terminates program when the last phase is over
class StatPhaseMgr {
	public:
		StatPhaseMgr();
		~StatPhaseMgr();

		void addPhase(StatPhase *ph);
		void start();

		StatPhase *operator ->() { return thePhase; }
		StatPhase *operator *() { return thePhase; }
		int phaseSyncPos() const;

		void forceFlush();
		void noteDone(StatPhase *ph);

		void reportCfg(ostream &os) const;

		int logCat() const { return theLogCat; }
		void logCat(int lc) { theLogCat = lc; }

		// whether we are waiting for the first transaction
		// returns false if called before start()
		bool trafficWaiting() const { return thePhaseIdx < 0 && theWaitPhase; }
		void stopTrafficWaiting();
		bool reachedNegative() const { return reachedNegative_; }

	protected:
		void nextPhase();

	protected:
		Array<StatPhase*> thePhases; // all phases are stored here
		StatPhase *thePhase; // current phase, thePhases[thePhaseIdx]
		StatPhase *theWaitPhase; // the first (a.k.a. "wait") phase
		int thePhaseIdx; // current phase index (negative before/while waiting)
		int theLogCat;      // log entry category

	private:
		bool reachedNegative_; // whether a phase reached a negative goal
};

extern StatPhaseMgr TheStatPhaseMgr;

#endif
