
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_GOALREC_H
#define POLYGRAPH__BASE_GOALREC_H

#include "base/LogObj.h"
#include "xstd/BigSize.h"

class String;

// used by GoalRec
class ErrGoalRec: public LogObj {
	public:
		ErrGoalRec();

		operator void*() const { return (theCount >= 0 || theRatio >= 0) ? (void*)-1 : 0; }
		Counter count() const { return theCount; }
		double ratio() const { return theRatio; }

		ErrGoalRec &operator +=(const ErrGoalRec &e);

		virtual OLog &store(OLog &) const;
		virtual ILog &load(ILog &);

		ostream &print(ostream &os) const;
		ostream &print(ostream &os, const String &pfx) const;

	public: /* for logging/config only */
		double theRatio;
		Counter theCount;
};

// log image of Goal
class GoalRec: public LogObj {
	public:
		GoalRec();

		operator void *() const; // true if positive goal is set
		Time duration() const { return theDuration; }
		Counter xactCnt() const { return theXactCnt; }
		const BigSize &fillSz() const { return theFillSz; }
		const ErrGoalRec &errs() const { return theErrs; }

		void concat(const GoalRec &g);
		void merge(const GoalRec &g);

		virtual OLog &store(OLog &) const;
		virtual ILog &load(ILog &);

		ostream &print(ostream &os) const;
		ostream &print(ostream &os, const String &pfx) const;

	protected:
		void join(const GoalRec &g);

	protected:
		Time theDuration;
		Counter theXactCnt;
		BigSize theFillSz;
		ErrGoalRec theErrs;
};

inline ostream &operator <<(ostream &os, const GoalRec &gr) { return gr.print(os); }

#endif
