
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_COMPOUNDXACTINFO_H
#define POLYGRAPH__RUNTIME_COMPOUNDXACTINFO_H

#include "base/CompoundXactStat.h"

class Xaction;

// information shared among several transactions
class CompoundXactInfo {
	public:
		static CompoundXactInfo *Create();
		static void Abandon(CompoundXactInfo *&info);

		Size reqSize; // accumulative size of requests
		Size repSize; // accumulative size of responses
		Time startTime; // when transaction started
		Time lifeTime; // full transaction duration
		unsigned int exchanges; // number of exchanges in transaction

		bool final; // true when compound transaction is completed

		inline void record(CompoundXactStat &s) const;

	protected:
		CompoundXactInfo();
		~CompoundXactInfo();

		void reset();

	private:
		CompoundXactInfo *theNext;
		static CompoundXactInfo *TheFree;
};

/* inlined methods */

inline
void CompoundXactInfo::record(CompoundXactStat &s) const {
	s.duration.record(lifeTime);
	s.reqSize.record(reqSize);
	s.repSize.record(repSize);
	s.exchanges.record(exchanges);
}

#endif
