
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "pgl/PglRec.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/VerFourAsSym.h"
#include "pgl/BenchSym.h"
#include "pgl/BenchSideSym.h"



const String VerFourAsSym::TheType = "VerFourAs";

static String strAgents_per_addr = "agents_per_addr";


VerFourAsSym::VerFourAsSym(const String &aType, PglRec *aRec): AddrSchemeSym(aType, aRec) {
	theRec->bAdd(IntSym::TheType, strAgents_per_addr, 0);
}

bool VerFourAsSym::agentsPerAddr(int &apa) const {
	return getInt(strAgents_per_addr, apa);
}

String VerFourAsSym::addresses(const BenchSideSym *side, ArraySym *&addrs) const {
	if (!side)
		return "bench side is not specified";

	if (SynSym *ams = side->addrMaskSym()) {
		cerr << ams->loc() << "warning: addressing schemes in fourth"
			<< " generation workloads use addr_space instead of addr_mask;"
			<< " consider removing depricated addr_mask from your new"
			<< " workloads" << endl;
	}

	const ArraySym *addrSpace = side->addrSpace();
	if (!addrSpace)
		return "addr_space not set";

	Assert(side->bench());
	double reqRate, hostLoad, agentLoad;
	int addrLoad;
	if (!side->bench()->peakReqRate(reqRate) || 
		!side->maxHostLoad(hostLoad) || 
		!side->maxAgentLoad(agentLoad) || agentLoad <= 0 ||
		!this->agentsPerAddr(addrLoad) || addrLoad <= 0)
		return "peak_req_rate, max_host_load, max_agent_load or agents_per_addr not set";

	const int maxAgentsPerHost = (int)xceil(hostLoad, agentLoad);
	const int maxAddrPerHost = (int)xceil(maxAgentsPerHost, addrLoad);

	// find the number of agents
	const int hostCnt = (int)xceil(reqRate, hostLoad);
	const int agentCnt = // divisible by addrLoad on each host
		singleDiv(hostCnt*addrLoad, doubleDiv(hostCnt, reqRate, agentLoad));
	if (agentCnt > addrLoad * maxAddrPerHost * hostCnt)
		return "too many agents?";

	if (agentCnt > addrSpace->count())
		return "too small address space";

	return distributeAddrs(addrs, addrSpace, hostCnt, agentCnt, addrLoad);
}

String VerFourAsSym::distributeAddrs(ArraySym *&addrs, const ArraySym *addrSpace, int hostCnt, int agentCnt, int addrLoad) const {
	const int agentPerHost = agentCnt / hostCnt;

	int minSubnet;
	if (const String err = this->minSubnet(agentPerHost/addrLoad, minSubnet))
		return err;

	Array<PglNetAddrRange*> ranges;
	int ipCount = 0;
	const int spaceSize = addrSpace->count();
	int maxSubnet = minSubnet;
	for (int h = 0, spaceIdx = 0; h < hostCnt && spaceIdx < spaceSize; ++h) {
		// one subnet, one host
		const NetAddrSym *lastAddr = 0;
		for (int a = 0; a < agentPerHost/addrLoad && spaceIdx < spaceSize; ++a) { // agents
			// one address per addrLoad agents
			lastAddr = &(const NetAddrSym&)addrSpace->item(spaceIdx++)->cast(NetAddrSym::TheType);
			const int thisSubnet = addAddr(ranges, *lastAddr, minSubnet);
			maxSubnet = Min(thisSubnet, maxSubnet); // "/24" <= "/16"
			ipCount++;
		}
		Assert(lastAddr);
		if (const String err = skipSubnet(lastAddr->val(), maxSubnet, addrSpace, spaceIdx))
			return err;
	}
	if (ipCount != agentCnt/addrLoad)
		return "addressing scheme cannot use provided addr_space";

	mergeRanges(ranges);
	addrs = rangesToAddrs(ranges, addrLoad);

	return String();
}

// skips address space that is in the [beg, end] subnet
String VerFourAsSym::skipSubnet(const NetAddr &lastAddr, int subnet, const ArraySym *addrSpace, int &spaceIdx) const {
	const int spaceSize = addrSpace->count();
	Assert(spaceIdx < spaceSize);
	for (; spaceIdx < spaceSize; ++spaceIdx) {
		const NetAddrSym *nas = &(const NetAddrSym&)addrSpace->item(spaceIdx)->cast(NetAddrSym::TheType);
		if (!sameSubnet(lastAddr, nas->val(), subnet))
			break;
	}
	return String(); 
}

// checks whether addr belongs to the [beg, end] subnet
bool VerFourAsSym::sameSubnet(const NetAddr &addr1, const NetAddr &addr2, int subnet) const {
	return addr1.addrN().sameSubnet(addr2.addrN(), subnet);
}

int VerFourAsSym::addAddr(Array<PglNetAddrRange*> &ranges, const NetAddrSym &addr, int subnet) const {
	NetAddrSym *clone = (NetAddrSym*)addr.clone();
	int explicitSubnet = -1;
	if (!clone->subnet(explicitSubnet))
		clone->setSubnet(subnet);
	else
	if (explicitSubnet <= subnet) // XXX: this logic is not IPv6-aware
		subnet = explicitSubnet;
	else {
		// Newer FreeBSD versions require (/32, /128) nmasks for aliases.
		// Warn, but do not quit.
		const int currentHash = explicitSubnet ^ subnet;
		static int warnLessHash = ~currentHash; // initialized once
		if (currentHash != warnLessHash) {
			warnLessHash = currentHash;
			clog << addr.loc() << "warning: the explicit subnet of the " << 
				addr << " address (/" << explicitSubnet << ") is 'smaller' " <<
				"than minimal subnet required to accommodate all agent " <<
				"addresses on one host (/" << subnet << "). Whether this " <<
				"is OK depends on your environment." << endl;
		}
	}

	addAddrToRanges(ranges, *clone);
	delete clone;

	return subnet;
}
