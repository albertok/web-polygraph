
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_CONTTYPESTAT_H
#define POLYGRAPH__BASE_CONTTYPESTAT_H

#include "xstd/Array.h"
#include "base/LogObj.h"

class String;
class AggrStat;

// groups stats for all close kinds
class ContTypeStat: public LogObj {
	public:
		static void RecordKind(int ctype, const String &kind);
		static const Array<String*> &Kinds() { return TheKinds; }
		static void Store(OLog &ol);
		static void Load(ILog &il);

	public:
		ContTypeStat();
		virtual ~ContTypeStat();

		AggrStat *hasStats(int idx) const { return (0 <= idx && idx < theStats.count()) ? theStats[idx] : 0; }
		AggrStat &stats(int idx) const { return *theStats[idx]; }

		void record(int ctype, Size sz);
		void add(const ContTypeStat &s);
		
		virtual OLog &store(OLog &ol) const;
		virtual ILog &load(ILog &il);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		static PtrArray<String*> TheKinds;

	protected:
		Array<AggrStat*> theStats;
};

#endif
