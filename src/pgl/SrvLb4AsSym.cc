
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglArraySym.h"
#include "pgl/BenchSym.h"
#include "pgl/BenchSideSym.h"
#include "pgl/SrvLb4AsSym.h"



const String SrvLb4AsSym::TheType = "SrvLb4As";

static String strSrvLb4 = "SrvLb-4";
static String strAddrArr = "addr[]";


SrvLb4AsSym::SrvLb4AsSym(): VerFourAsSym(TheType, new PglRec) {
	kind(strSrvLb4);
}

SrvLb4AsSym::SrvLb4AsSym(const String &aType, PglRec *aRec): VerFourAsSym(aType, aRec) {
}

bool SrvLb4AsSym::isA(const String &type) const {
	return VerFourAsSym::isA(type) || type == TheType;
}

SynSym *SrvLb4AsSym::dupe(const String &type) const {
	if (isA(type))
		return new SrvLb4AsSym(this->type(), theRec->clone());
	return VerFourAsSym::dupe(type);
}

String SrvLb4AsSym::robots(ArraySym *&addrs) const {
	return addresses(theBench->clientSide(), addrs);
}

String SrvLb4AsSym::servers(ArraySym *&addrs) const {
	if (const ArraySym *space = theBench->serverSide()->addrSpace()) {
		cerr << space->loc() << "warning: SrvLb4As addressing scheme"
			<< " uses server_side.hosts only; your setting of"
			<< " server_side.addr_space will be ignored" << endl;
	}

	if (ArraySym *hosts = theBench->serverSide()->hostsSym()) {
		if (hosts->count()) {
			addrs = &((ArraySym&)hosts->clone()->cast(strAddrArr));
			return String();
		}
	}

	return "no server_side.hosts specified";
}

String SrvLb4AsSym::proxies(ArraySym *&addrs) const {
	return VerFourAsSym::proxies(addrs);
}
