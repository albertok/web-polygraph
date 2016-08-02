
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_STATINTVL_H
#define POLYGRAPH__RUNTIME_STATINTVL_H

#include "xstd/AlarmClock.h"
//#include "polyBcastChannels.h"
#include "runtime/BcastRcver.h"

class Agent;
class Connection;
class Xaction;
class IcpCltXact;
class StatIntvlRec;

// collects and aggregates simple stats in StatIntvlRec
// base class for StatCycle, StatPhase, and DutWatchdog
class StatIntvl: public AlarmUser, public BcastRcver {
	public:
		static void ActiveCat(int cat);
		static int TheReportCat; // which log category to report

	public:
		StatIntvl();

		virtual void noteAgentEvent(BcastChannel *ch, const Agent *a);
		virtual void noteConnEvent(BcastChannel *ch, const Connection *c);
		virtual void noteXactEvent(BcastChannel *ch, const Xaction *x);
		virtual void noteCompoundXactEvent(BcastChannel *ch, const CompoundXactInfo *cx);
		virtual void noteIcpXactEvent(BcastChannel *ch, const IcpXaction *x);
		virtual void notePageEvent(BcastChannel *ch, const PageInfo *p);

		Time startTime() const { return theIntvlStart; }

		// these should check that cat is not lgcAll
		virtual StatIntvlRec &getRec(int cat) = 0;
		virtual const StatIntvlRec &getRec(int cat) const = 0;

		static void UpdateLiveStats(); // recalculate live stats, if needed

	protected:
		// returns true iff all processing should stop (XXX)
		virtual bool checkpoint(); // called after event is handled

		void setDuration(Time start);
		void noteBusyConnEnd(StatIntvlRec &rec, const Connection *conn);
		void storeAll(OLog &ol, int tag) const;
		
		void report(ostream &os) const;

	protected:
		static Array<bool> IsActiveCat;

		Time theIntvlStart;
};

#endif
