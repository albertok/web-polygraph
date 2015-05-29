
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBELINKSTAT_H
#define POLYGRAPH__PROBE_PROBELINKSTAT_H

#include "xstd/Error.h"
#include "xstd/Time.h"
#include "base/histograms.h"

class String;

class ProbeLinkStat {
	public:
		ProbeLinkStat();

		void recordConn();
		void recordError(const Error &err);
		void recordRead(Size sz);
		void recordWrite(Size sz);

		Time duration() const { return theEnd - theStart; }

		void syncWith(const ProbeLinkStat &s);
		ProbeLinkStat &operator +=(const ProbeLinkStat &s);

		void store(OLog &log) const;
		void load(ILog &log);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		void syncDur();

	public: /* read-only */
		SizeHist theSockRdSzH;   // socket reads
		SizeHist theSockWrSzH;   // socket writes
		
		int theConnCnt;  // number of established conn
		int theErrorCnt; // various errors

		Time theStart;
		Time theEnd;
};

#endif
