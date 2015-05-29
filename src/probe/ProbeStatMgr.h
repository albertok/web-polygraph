
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBESTATMGR_H
#define POLYGRAPH__PROBE_PROBESTATMGR_H

#include "xstd/Array.h"
#include "probe/ProbeLinkStat.h"

class ProbeLinkRec {
	public:
		ProbeLinkRec();
		ProbeLinkRec(const NetAddr &aCltHost, const NetAddr &aSrvHost);

		const NetAddr &cltHost() const { return theCltHost; }
		const NetAddr &srvHost() const { return theSrvHost; }

		ProbeLinkStat &stats() { return theStats; }
		const ProbeLinkStat &stats() const { return theStats; }

		void oneLineReport(ostream &os) const;

		void store(OLog &log) const;
		void load(ILog &log);

	protected:
		String fmtAddress(const NetAddr &addr) const;
		double bitRate(const AggrStat &s) const;

	protected:
		NetAddr theCltHost;
		NetAddr theSrvHost;
		ProbeLinkStat theStats;
};

class ProbeStatMgr {
	public:
		ProbeStatMgr();
		~ProbeStatMgr();

		void incConfigure(Array<NetAddr*> &cltHosts, Array<NetAddr*> &srvHosts);

		int linkCount() const { return theRecs.count(); }

		void exportStats(OLog &log) const;
		void importStats(ILog &log);

		ProbeLinkStat *stats(const NetAddr &cltHost, const NetAddr &srvHost);

		void report(ostream &os) const;

	protected:
		ProbeLinkStat *find(const NetAddr &cltHost, const NetAddr &srvHost);
		void report(ostream &os, const ProbeLinkRec &rec, int idx = -1) const;

		void importStats(ProbeLinkRec *rec);

	protected:
		Array<ProbeLinkRec*> theRecs;
};

extern ProbeStatMgr TheProbeStatMgr;

#endif
