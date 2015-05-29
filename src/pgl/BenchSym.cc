
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "pgl/PglRec.h"
#include "pgl/PglRateSym.h"
#include "pgl/BenchSideSym.h"
#include "pgl/BenchSym.h"



String BenchSym::TheType = "Bench";

static String strClient_side = "client_side";
static String strProxy_side = "proxy_side";
static String strServer_side = "server_side";
static String strPeak_req_rate = "peak_req_rate";


BenchSym::BenchSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(RateSym::TheType, strPeak_req_rate, 0);
	theRec->bAdd(BenchSideSym::TheType, strClient_side, new BenchSideSym);
	theRec->bAdd(BenchSideSym::TheType, strServer_side, new BenchSideSym);
	theRec->bAdd(BenchSideSym::TheType, strProxy_side, new BenchSideSym);
}

BenchSym::BenchSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool BenchSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *BenchSym::dupe(const String &type) const {
	if (isA(type))
		return new BenchSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

bool BenchSym::peakReqRate(double &rate) const {
	return getRate(strPeak_req_rate, rate);
}

const BenchSideSym *BenchSym::clientSide() const {
	return getSide(strClient_side);
}

const BenchSideSym *BenchSym::serverSide() const {
	return getSide(strServer_side);
}

const BenchSideSym *BenchSym::proxySide() const {
	return getSide(strProxy_side);
}

const BenchSideSym *BenchSym::side(const String &name) const {
	return getSide(name + "_side");
}

String BenchSym::clientHostCount(int &count) const {
	const BenchSideSym *side = clientSide();
	if (!side)
		return "client side of the bench is not configured";

	double peakRate;
	if (!peakReqRate(peakRate))
		return "peak_req_rate is not set for the bench";
	if (peakRate <= 0)
		return "peak_req_rate is not positive for the bench";

	double hostLoad;
	if (!side->maxHostLoad(hostLoad))
		return "max_host_load is not set for the client side of the bench";
	if (hostLoad <= 0)
		return "max_host_load is not positive for the client side of the bench";

	count = (int)xceil(peakRate, hostLoad);
	return String();
}

const BenchSideSym *BenchSym::getSide(const String &name) const {
	SynSymTblItem *si = 0;
	Assert(theRec);
	Assert(theRec->find(name, si));
	if (!si->sym())
		return 0;

	const BenchSideSym *side = &(const BenchSideSym&)si->sym()->cast(BenchSideSym::TheType);
	side->bench(this);
	return side;
}
