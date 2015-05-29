
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_CONNCLOSESTAT_H
#define POLYGRAPH__BASE_CONNCLOSESTAT_H

#include "xstd/Array.h"
#include "base/LogObj.h"
#include "base/histograms.h"

class ConnCloseStatItem;

// groups stats for all close kinds
class ConnCloseStat: public LogObj {
	public:
		typedef enum { ckNone, ckBusy, ckIdleLocal, ckIdleForeign, ckEnd } CloseKind;

	public:
		ConnCloseStat();
		virtual ~ConnCloseStat();

		void useHist(Log2Hist &acc) const; // totals
		void ttlHist(TimeHist &acc) const;
		const Log2Hist &useHist(int kind) const; // specifics
		const TimeHist &ttlHist(int kind) const;

		void record(CloseKind kind, Time ttl, int useCnt);
		void add(const ConnCloseStat &s);
		
		virtual OLog &store(OLog &ol) const;
		virtual ILog &load(ILog &il);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		Array<ConnCloseStatItem*> theStats;
};

#endif
