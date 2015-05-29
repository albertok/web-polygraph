
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "pgl/PglRec.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/VerThreeAsSym.h"
#include "pgl/BenchSym.h"
#include "pgl/BenchSideSym.h"



const String VerThreeAsSym::TheType = "VerThreeAs";



VerThreeAsSym::VerThreeAsSym(const String &aType, PglRec *aRec): AddrSchemeSym(aType, aRec) {
}

String VerThreeAsSym::addresses(const BenchSideSym *side, int shift, ArraySym *&addrs) const {
	if (!side)
		return "bench side is not configured";

	const NetAddrSym *mask = side->addrMaskSym();
	if (!mask)
		return "address mask is not set";

	Assert(side->bench());
	double reqRate, hostLoad, agentLoad;
	if (!side->bench()->peakReqRate(reqRate) || 
		!side->maxHostLoad(hostLoad) || !side->maxAgentLoad(agentLoad) || 
		agentLoad <= 0) 
		return "peak_req_rate, max_host_load, or max_agent_load not set";

	const int hostCount = (int)xceil(reqRate, hostLoad);

	const int rbtCntApx = doubleDiv(hostCount, reqRate, agentLoad);
	const int srvCntApx = singleDiv(hostCount, rbtCntApx*0.1 + 500);
	const int agentCntApx = shift ? srvCntApx : rbtCntApx;

	const int maxX = doubleDiv(hostCount, agentCntApx, 250);
	const int maxY = (int)xceil(agentCntApx, maxX);

	const String sideStr = ipRangeToStr(*mask, shift+1, maxX, 1, maxY);
	addrs = ipStrToArr(sideStr);
	return String();
}

ArraySym *VerThreeAsSym::ipStrToArr(const String &str) const {
	PglNetAddrRange ar;
	if (!ar.parse(str))
		return 0;

	return ar.toSyms(loc());
}
