
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "xstd/String.h"
#include "xstd/Socket.h"
#include "xstd/Ssl.h"
#include "base/RndPermut.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/Agent.h"
#include "runtime/AgentCfg.h"
#include "runtime/PortMgr.h"
#include "runtime/globals.h"
#include "pgl/AgentSym.h"
#include "pgl/SocketSym.h"

int Agent::TheLastSeqvId = 0;

Agent::Agent(): isCookieSender(false), theLifetimeDistr(0), theThinkDistr(0),
	theSslCtx(0), theCache(0) {
	theSeqvId = ++TheLastSeqvId;
}

Agent::~Agent() {
	delete theSslCtx;
}

void Agent::configure(const AgentSym *cfg, const NetAddr &aHost) {
	Assert(TheFileScanner);
	Assert(cfg);

	theHost = aHost;
	theKind = cfg->kind();

	if (UniqId id = cfg->world())
		theId = id;
	else
		theId.create();

	if (!theLifetimeDistr) {
		theLifetimeDistr = cfg->xactLifetime();
		if (theLifetimeDistr)
			theLifetimeDistr->rndGen(LclRndGen("agent_xact_lifetime"));
	}

	if (!theThinkDistr) {
		theThinkDistr = cfg->xactThink();
		if (theThinkDistr)
			theThinkDistr->rndGen(LclRndGen("agent_xact_think"));
	}

	// socket options
	bool ng = true;
	if (cfg->socket()->nagle(ng))
		theSockOpt.nagle = ng ? 1 : -1;
	cfg->socket()->lingerTout(theSockOpt.linger);

	selectHttpVersion();
}

void Agent::cache(Cache *aCache) {
	Assert(!theCache);
	theCache = aCache;
}

Time Agent::selectLifetime() const {
	return theLifetimeDistr ? Time::Secd(theLifetimeDistr->trial()) : Time();
}

void Agent::selectHttpVersion() {
	switch (cfg()->selectHttpVersion()) {
		case protoHttp1p0:
			theHttpVersion = HttpVersion(1, 0);
			return;
		case protoHttp1p1:
			theHttpVersion = HttpVersion(1, 1);
			return;
		default:
			Assert(false);
	}
}

void Agent::storeWorkingSet(OBStream &os) {
	os << theId;
}

void Agent::loadWorkingSet(IBStream &is) {
	is >> theId;
}

void Agent::missWorkingSet() {
}

void Agent::start() {
	Broadcast(TheAgentBegChannel, this);
}

void Agent::stop() {
	Broadcast(TheAgentEndChannel, this);
}

void Agent::describe(ostream &os) const {
	if (theKind)
		os << theKind << ' ';
	os << '[' << seqvId() << " / " << theId << "] at " << theHost;

	if (theHttpVersion.known())
		os << " HTTP/" << theHttpVersion.vMajor() << '.' << theHttpVersion.vMinor();
	if (theSslCtx)
		os << " SSL";
}

Socket Agent::makeListenSocket(const NetAddr &addr) {
	Socket s(makeSocket(addr));
	if (!s || !s.bind(addr) || !s.listen())
		s.close();
	return s;
}

Socket Agent::makeListenSocket(PortMgr *const portMgr) {
	Assert(portMgr);
	Socket s(makeSocket(portMgr->addr()));
	if (!s || (portMgr->bind(s) < 0) || !s.listen())
		s.close();
	return s;
}

Socket Agent::makeSocket(const NetAddr &addr) {
	Socket s;

	if (!s.create(addr.addrN().family()))
		return s;

	if (s.blocking(false)) {
		s.reuseAddr(true);
		s.configure(theSockOpt);
	}

	return s;
}
