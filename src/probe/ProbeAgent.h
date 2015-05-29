
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBEAGENT_H
#define POLYGRAPH__PROBE_PROBEAGENT_H

#include "xstd/NetAddr.h"
#include "xstd/Socket.h"
#include "xstd/FileScanner.h"
#include "runtime/BcastRcver.h"

class ProbeXact;

class ProbeAgent: public FileScanner::User, public BcastRcver {
	public:
		ProbeAgent(const NetAddr &aLclHost, const NetAddr &aRmtHost);
		virtual ~ProbeAgent();

		const NetAddr &lclHost() const { return theLclHost; }
		const NetAddr &rmtHost() const { return theRmtHost; }

		virtual void exec() = 0;

		void noteXactDone(ProbeXact *x);

	protected:
		void setSockOpt(Socket &s);

		virtual void noteInfoEvent(BcastChannel *ch, InfoEvent ev);

	protected:
		NetAddr theLclHost;
		NetAddr theRmtHost;

		Socket theSock;
		FileScanReserv theReserv;
		bool isDone;
};

#endif
