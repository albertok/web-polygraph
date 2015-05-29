
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/BenchSideSym.h"
#include "pgl/BenchSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglRec.h"
#include "xstd/gadgets.h"

#include "pgl/SpreadAsSym.h"


const String SpreadAsSym::TheType = "SpreadAs";

static const String strSpread = "Spread";
static const String strAgents_per_addr = "agents_per_addr";


SpreadAsSym::SpreadAsSym(): AddrSchemeSym(TheType, new PglRec) {
	kind(strSpread);
	theRec->bAdd(IntSym::TheType, strAgents_per_addr, 0);
}

SpreadAsSym::SpreadAsSym(const String &aType, PglRec *aRec): AddrSchemeSym(aType, aRec) {
	theRec->bAdd(IntSym::TheType, strAgents_per_addr, 0);
}

bool SpreadAsSym::isA(const String &type) const {
	return AddrSchemeSym::isA(type) || type == TheType;
}

SynSym *SpreadAsSym::dupe(const String &type) const {
	if (isA(type))
		return new SpreadAsSym(this->type(), theRec->clone());
	return AddrSchemeSym::dupe(type);
}

bool SpreadAsSym::agentsPerAddr(int &apa) const {
	return getInt(strAgents_per_addr, apa);
}

String SpreadAsSym::robots(ArraySym *&addrs) const {
	return addresses(theBench->clientSide(), addrs);
}

String SpreadAsSym::servers(ArraySym *&addrs) const {
	return addresses(theBench->serverSide(), addrs);
}

String SpreadAsSym::addresses(const BenchSideSym *side, ArraySym *&addrs) const {
	if (!side)
		return "bench side is not specified";

	const ArraySym *const addrSpace = side->addrSpace();
	if (!addrSpace)
		return "addr_space not set";

	Array<NetAddr*> hosts;
	if (!side->hosts(hosts) || hosts.empty())
		return "hosts not set";

	Assert(side->bench());
	double reqRate, agentLoad;
	int addrLoad;
	if (!side->bench()->peakReqRate(reqRate) ||
		!side->maxAgentLoad(agentLoad) || agentLoad <= 0 ||
		!this->agentsPerAddr(addrLoad) || addrLoad <= 0)
		return "peak_req_rate, max_agent_load or agents_per_addr not set";

	// server-side addrLoad is always 1
	if (side == theBench->serverSide())
		addrLoad = 1;

	const int hostCnt = hosts.count();

	// find the number of agents
	const int agentCnt = // divisible by addrLoad on each host
		doubleDiv(hostCnt*addrLoad, reqRate, agentLoad);
	if (agentCnt / addrLoad > addrSpace->count())
		return "too small address space";

	// more checks if max_host_load is set
	double maxHostLoad;
	if (side->maxHostLoad(maxHostLoad)) {
		// max_host_load vs peak_req_rate
		const double hostLoad = reqRate / hostCnt;
		if (hostLoad > maxHostLoad) {
			cerr << loc() << "warning: actual host load (" << hostLoad
				<< ") is higher that max_host_load (" << maxHostLoad
				<< ')' << endl;
		}

		// max_host_load vs max_agent_load
		const int maxAgentsPerHost = (int)xceil(maxHostLoad, agentLoad);
		const int maxAddrPerHost = (int)xceil(maxAgentsPerHost, addrLoad);
		if (agentCnt > addrLoad * maxAddrPerHost * hostCnt)
			return "too many agents?";
	}

	distributeAddrs(addrs, addrSpace, hostCnt, agentCnt, addrLoad);

	return String();
}

void SpreadAsSym::distributeAddrs(ArraySym *&addrs, const ArraySym *addrSpace, int hostCnt, int agentCnt, int addrLoad) const {
	const int addrsPerHost = agentCnt / hostCnt / addrLoad;
	const int hostSlice = addrSpace->count() / hostCnt;

	Array<PglNetAddrRange*> ranges;
	for (int h = 0; h < hostCnt; ++h) {
		for (int i = 0; i < addrsPerHost; ++i) {
			const NetAddrSym *const addr =
				&(const NetAddrSym&)addrSpace->item(i + hostSlice * h)->cast(NetAddrSym::TheType);
			addAddrToRanges(ranges, *addr);
		}
	}

	mergeRanges(ranges);
	addrs = rangesToAddrs(ranges, addrLoad);
}
