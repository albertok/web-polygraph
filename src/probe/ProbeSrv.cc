
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "probe/ProbeSrv.h"
#include "probe/ProbeSrvXact.h"
#include "probe/ProbeStatMgr.h"
#include "probe/PolyProbe.h"

ProbeSrv::ProbeSrv(const NetAddr &aLclHost): ProbeAgent(aLclHost, NetAddr()) {
}

void ProbeSrv::exec() {
	ProbeAgent::exec();

	// setup listen socket
	Must(theSock.create(theLclHost.addrN().family()));
	setSockOpt(theSock);
	Must(theSock.reuseAddr(true));
	Must(theSock.bind(theLclHost));
	Must(theSock.listen());

	theReserv = TheFileScanner->setFD(theSock.fd(), dirRead, this);	
	clog << theLclHost << ": listening" << endl;
}

void ProbeSrv::noteXactDone(ProbeSrvXact *x) {
	ProbeAgent::noteXactDone(x);
	delete x;
}

void ProbeSrv::noteReadReady(int fd) {
	Assert(fd == theSock.fd()); // remove later
	Assert(!isDone);
	
	NetAddr rmtAddr;
	Socket newSock = theSock.accept(rmtAddr);
	if (!newSock) {
		const Error err = Error::Last();
		const bool fatal = err != EWOULDBLOCK && err != EAGAIN && err != EMFILE;
		if (fatal)
			cerr << "failed to accept a connection: " << err << endl << xexit;
	}

	rmtAddr.port(-1); // rport may be random, disable to limit number of links
	TheProbeStatMgr.stats(rmtAddr, theLclHost)->recordConn();
	setSockOpt(newSock);
	ProbeSrvXact *x = new ProbeSrvXact(rmtAddr, lclHost(), newSock.fd());
	x->exec(this);
	// unsafe to call anything else as exec might return
}
