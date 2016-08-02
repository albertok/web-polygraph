
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_COMPOUNDXACTINFO_H
#define POLYGRAPH__RUNTIME_COMPOUNDXACTINFO_H

#include "base/CompoundXactStat.h"
#include "base/AuthPhaseStat.h"

class Xaction;

// information shared among several transactions
class CompoundXactInfo {
	public:
		static CompoundXactInfo *Create(const int logCat);
		static void Abandon(CompoundXactInfo *&info);
		static void Share(CompoundXactInfo *info);

		int logCat; // the logging "side" of individual transactions
		int ccLevel; // number of alive individual transactions

		Size reqSize; // accumulative size of requests
		Size repSize; // accumulative size of responses
		Time startTime; // when the first individual transaction started
		Time finishTime; // when the last individual transaction finished
		unsigned int exchanges; // number of finished individual transactions

		typedef enum { opNone, opIng, opDone } OpState; // operation progress
		OpState connectState; // HTTP CONNECT progress
		OpState proxyAuthState; // proxy authentication progress
		AuthPhaseStat::Scheme proxyStatAuth; // authentication state

		// not expecting any new individual transactions
		bool completed() const { return connectState != opIng && proxyAuthState != opIng; }

		void record(CompoundXactStat &s) const;
		void reset();

	protected:
		CompoundXactInfo();
		~CompoundXactInfo();

	private:
		CompoundXactInfo *theNext;
		static CompoundXactInfo *TheFree;
};

#endif
