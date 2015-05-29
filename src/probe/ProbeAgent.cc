
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/polyBcastChannels.h"
#include "probe/ProbeAgent.h"
#include "probe/ProbeXact.h"
#include "probe/PolyProbe.h"

ProbeAgent::ProbeAgent(const NetAddr &aLclHost, const NetAddr &aRmtHost):
	theLclHost(aLclHost), theRmtHost(aRmtHost), isDone(false) {
	Assert(theLclHost);
	//Assert(theLclHost != theRmtHost);
}

ProbeAgent::~ProbeAgent() {
	if (theReserv)
		TheFileScanner->clearRes(theReserv);
}

void ProbeAgent::exec() {
	theChannels.append(ThePhasesEndChannel);
	startListen();
}

void ProbeAgent::noteXactDone(ProbeXact *x) {
	delete x;
}

void ProbeAgent::noteInfoEvent(BcastChannel *ch, InfoEvent ev) {
	Assert(ch == ThePhasesEndChannel);
	Assert(ev == BcastRcver::ieNone);
	isDone = true;
	if (theSock.fd() >= 0) {
		theSock.close();
		TheFileScanner->clearRes(theReserv);
	}
}

void ProbeAgent::setSockOpt(Socket &s) {
	SockOpt opt;
	opt.nagle = -1;
	Must(s.blocking(false));
	Should(s.reuseAddr(true));
	Must(s.configure(opt));
}
