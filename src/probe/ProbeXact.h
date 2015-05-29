
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBEXACT_H
#define POLYGRAPH__PROBE_PROBEXACT_H

#include "xstd/NetAddr.h"
#include "xstd/Socket.h"
#include "xstd/FileScanner.h"
#include "runtime/BcastRcver.h"

class ProbeAgent;
class ProbeLinkStat;

class ProbeXact: public FileScanner::User, public BcastRcver {
	public:
		ProbeXact(const NetAddr &aCltHost, const NetAddr &aSrvHost, int fd);
		virtual ~ProbeXact();

		virtual ProbeAgent *owner() = 0;
		virtual const ProbeAgent *owner() const = 0;

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);

		ProbeLinkStat *stats() { return theStats; }

	protected:
		void exec();
		virtual void finish(const Error &err);

		const NetAddr &cltHost() const { return theCltHost; }
		const NetAddr &srvHost() const { return theSrvHost; }

		virtual void noteInfoEvent(BcastChannel *ch, InfoEvent ev);

	protected:
		NetAddr theCltHost;
		NetAddr theSrvHost;

		Socket theSock;
		FileScanReserv theReadR;
		FileScanReserv theWriteR;

		ProbeLinkStat *theStats;
		int theRdCount;
};

#endif
