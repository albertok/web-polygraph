
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_OIDGENSTAT_H
#define POLYGRAPH__BASE_OIDGENSTAT_H

#include "xstd/h/iosfwd.h"
#include "xstd/String.h"

class OLog;
class ILog;

// oid generation statistics

class OidGenStat {
	public:
		enum Interest { intNone = 0, intPrivate, intPublic, intForeign };

	protected:
		typedef Counter Table[8];

	public:
		OidGenStat();

		void reset();

		// these two must be called in the listed order
		void recordNeed(bool rep, int world);
		void recordGen(bool rep, int world);

		void store(OLog &log) const;
		void load(ILog &log);

		OidGenStat &operator +=(const OidGenStat &s);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		inline int toIdx(bool rep, int world) const;

		void mergeTable(Table &a, const Table &b) const;
		void writeTable(OLog &log, const Table &t) const;
		void readTable(ILog &log, Table &t);
		void printTable(ostream &os, const String &pfx, const Table &t) const;

	protected:
		Table theNeed;
		Table theLack;
		Table theGen;

	private:                 /* cached need info */
		int needWorld;
		bool needRep;
		bool waitForGen;
};

inline
int OidGenStat::toIdx(bool rep, int world) const {
	return world | (rep ? 4 : 0);
}


inline
ostream &operator <<(ostream &os, const OidGenStat &s) { return s.print(os, ""); }

inline
OLog &operator <<(OLog &log, const OidGenStat &s) { s.store(log); return log; }

inline
ILog &operator >>(ILog &log, OidGenStat &s) { s.load(log); return log; }

#endif
