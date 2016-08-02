
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_STATSSAMPLEMGR_H
#define POLYGRAPH__RUNTIME_STATSSAMPLEMGR_H

#include "xstd/Array.h"
#include "xstd/AlarmClock.h"

class StatsSampleSym;
class StatsSampleCfg;

// schedules and initiates sampling
class StatsSampleMgr: public AlarmUser {
	public:
		StatsSampleMgr();
		virtual ~StatsSampleMgr();

		void configure(const Array<StatsSampleSym*> &cfgs);
		void addSample(const StatsSampleCfg &cfg);
		void willAddSample();

		void start();

		virtual void wakeUp(const Alarm &);

		void reportCfg(ostream &os) const;

	protected:
		void schedSample(const StatsSampleCfg &cfg);
		void startSample();

	protected:
		Array<StatsSampleCfg*> theCfgs;
		int theStartCnt;     // how many samples were started
		int thePendCfgCnt;   // how many cfgs will be addeded
		bool didStart; // start() has been called already
};

extern StatsSampleMgr TheStatsSampleMgr;

#endif
