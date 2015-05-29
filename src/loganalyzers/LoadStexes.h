
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_LOADSTEXES_H
#define POLYGRAPH__LOGANALYZERS_LOADSTEXES_H

#include "loganalyzers/LoadStex.h"

// total side  load (all requests or all responses)
class SideLoadStex: public LoadStex {
	public:
		typedef double (StatIntvlRec::*StatPtr)() const;

	public:
		SideLoadStex(const String &aKey, const String &aName, StatPtr aRate, StatPtr aBwidth);

		virtual double rate(const StatIntvlRec &rec) const;
		virtual double bwidth(const StatIntvlRec &rec) const;

	protected:
		StatPtr theRateStats;
		StatPtr theBwidthStats;
};

// load based on TmSzStex
class TmSzLoadStex: public LoadStex {
	public:
		TmSzLoadStex(const Stex *aStex);

		virtual double rate(const StatIntvlRec &rec) const;
		virtual double bwidth(const StatIntvlRec &rec) const;

	protected:
		const Stex *theStex;
};


#if FUTURE_CODE
// load based on TmSzStat data
class TmSzLoadStex: public LoadStex {
	public:
		typedef TmSzStat (StatIntvlRec::*StatPtr);

	public:
		TmSzLoadStex(const String &aKey, const String &aName, StatPtr aStats):
			LoadStex(aKey, aName), theStats(aStats) {}

		virtual double rate(const StatIntvlRec &rec) const;
		virtual double bwidth(const StatIntvlRec &rec) const;

	protected:
		StatPtr theStats;
};

// load based on HRStat data
class HrLoadStex: public LoadStex {
	public:
		typedef HRStat (StatIntvlRec::*StatPtr);

	public:
		HrLoadStex(const String &aKey, const String &aName, StatPtr aStats):
			LoadStex(aKey, aName), theStats(aStats) {}

		virtual double rate(const StatIntvlRec &rec) const;
		virtual double bwidth(const StatIntvlRec &rec) const;

	protected:
		StatPtr theStats;
};
#endif

// total protocol-specific side load (all protocol messages)
class ProtoSideLoadStex: public LoadStex {
	public:
		typedef double (ProtoIntvlStat::*StatPtr)(Time) const;
		typedef ProtoIntvlStat StatIntvlRec::*ProtoPtr;

	public:
		ProtoSideLoadStex(const String &aKey, const String &aName,
			ProtoPtr aProto, StatPtr aRate, StatPtr aBwidth);

		virtual double rate(const StatIntvlRec &rec) const;
		virtual double bwidth(const StatIntvlRec &rec) const;

	protected:
		ProtoPtr theProto;
		StatPtr theRateStats;
		StatPtr theBwidthStats;
};

#endif
