
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_POLYPROBE_H
#define POLYGRAPH__PROBE_POLYPROBE_H

#include "xstd/NetAddr.h"
#include "xstd/String.h"
#include "xstd/Array.h"
#include "xstd/FileScanner.h"
#include "xstd/AlarmClock.h"
#include "base/CmdLine.h"
#include "probe/ProbeSrv.h"

class ProbeAgent;
class ProbeClt;
class ProbeSrv;
class ProbeLinkRec;

// exhanges simple messages with in mesh of like applications
// to measure "raw" network performance of multiple-host benches
class PolyProbe: public AlarmUser, protected FileScanner::User {
	public:
		PolyProbe();
		virtual ~PolyProbe();

		int run(int argc, char *argv[]);
		void sendStats(Socket &s, const NetAddr &to);

		virtual void wakeUp(const Alarm &a);

	protected:
		void start();
		void step(Time tout);

		bool handleCmdLine(int argc, char *argv[]);
		void configureLogs(int prec);
		void configureHosts();
		void configure();
		void setResourceLimits();

		void configureHosts(const Array<String*> &ranges, Array<NetAddr*> &allHosts, Array<NetAddr*> &lclHosts);
		void dumpHostSpace(const Array<NetAddr*> &clients, const Array<NetAddr*> &servers);
		void dumpHosts(ostream &os, const Array<InAddress> &hosts);
		void dumpHosts(ostream &os, const Array<NetAddr*> &hosts);

		void pullStats();

		void reportCfg() const;
		void reportStats() const;

	protected:
		CmdLine theCmdLine;
		String thePrgName;

		Array<NetAddr*> theAllClients;
		Array<NetAddr*> theAllServers;

		Array<ProbeClt*> theClients;
		Array<ProbeSrv*> theServers;
		Array<ProbeAgent*> theAgents;

		bool exchangingStats;
		bool mustStop;
};

extern PolyProbe *ThePolyProbe;
extern FileScanner *TheFileScanner;


#endif
