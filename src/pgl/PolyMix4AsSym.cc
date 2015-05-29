
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "base/AnyToString.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/PglNetAddrRangeSym.h"
#include "pgl/BenchSym.h"
#include "pgl/BenchSideSym.h"
#include "pgl/PolyMix4AsSym.h"



const String PolyMix4AsSym::TheType = "PolyMix4As";

static const String strPolyMix4 = "PolyMix-4";


PolyMix4AsSym::PolyMix4AsSym(): VerFourAsSym(TheType, new PglRec) {
	kind(strPolyMix4);
}

PolyMix4AsSym::PolyMix4AsSym(const String &aType, PglRec *aRec): VerFourAsSym(aType, aRec) {
}

bool PolyMix4AsSym::isA(const String &type) const {
	return VerFourAsSym::isA(type) || type == TheType;
}

SynSym *PolyMix4AsSym::dupe(const String &type) const {
	if (isA(type))
		return new PolyMix4AsSym(this->type(), theRec->clone());
	return VerFourAsSym::dupe(type);
}

String PolyMix4AsSym::robots(ArraySym *&addrs) const {
	return addresses(theBench->clientSide(), addrs);
}

String PolyMix4AsSym::servers(ArraySym *&addrs) const {
	ArraySym *r = 0;
	if (const String err = robots(r))
		return err;
	const int minSrvCount = (int)(500.5 + 0.1*r->count());
	delete r;

	const BenchSideSym *side = theBench->serverSide();
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
	double reqRate, hostLoad;
	if (!side->bench()->peakReqRate(reqRate) || 
		!side->maxHostLoad(hostLoad))
		return "peak_req_rate, max_host_load, max_agent_load or agents_per_addr not set";

	double agentLoad;
	if (side->maxAgentLoad(agentLoad)) {
		cerr << loc() << "warning: PolyMix-4 addressing scheme does not"
			<< " use server_side.max_agent_load, but the bench has"
			<< " server_side.max_agent_load defined" << endl;
		if (side->loc())
			cerr << endl << side->loc() << "warning: server_side.max_agent_load"
				<< " is defined here" << endl;
	}

	const int hostCnt = (int)xceil(reqRate, hostLoad);
	const int agentCnt = singleDiv(hostCnt, minSrvCount);
	if (agentCnt > addrSpace->count()) {
		return "too small address space; " + AnyToString(agentCnt) +
			" agents cannot fit into "  + AnyToString(addrSpace->count()) +
			" space slots";
	}

	// always use addrLoad of 1 for servers
	return distributeAddrs(addrs, addrSpace, hostCnt, agentCnt, 1);
}

String PolyMix4AsSym::proxies(ArraySym *&addrs) const {
	return addresses(theBench->proxySide(), addrs);
}
