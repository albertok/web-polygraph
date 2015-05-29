
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/BenchSym.h"
#include "pgl/BenchSideSym.h"
#include "pgl/WebAxe4AsSym.h"



const String WebAxe4AsSym::TheType = "WebAxe4As";

static String strAddrArr = "addr[]";

static const String strWebAxe4 = "WebAxe-4";

WebAxe4AsSym::WebAxe4AsSym(): VerFourAsSym(TheType, new PglRec) {
	kind(strWebAxe4);
}

WebAxe4AsSym::WebAxe4AsSym(const String &aType, PglRec *aRec): VerFourAsSym(aType, aRec) {
}

bool WebAxe4AsSym::isA(const String &type) const {
	return VerFourAsSym::isA(type) || type == TheType;
}

SynSym *WebAxe4AsSym::dupe(const String &type) const {
	if (isA(type))
		return new WebAxe4AsSym(this->type(), theRec->clone());
	return VerFourAsSym::dupe(type);
}

String WebAxe4AsSym::robots(ArraySym *&addrs) const {
	return addresses(theBench->clientSide(), addrs);
}

String WebAxe4AsSym::servers(ArraySym *&addrs) const {
	if (theBench->serverSide()->addrSpace()) {
		cerr << loc() << "warning: WebAxe4As addressing scheme "
			<< " uses server_side.hosts only; your setting of "
			<< " server_side.addr_space will be ignored" << endl;
	}

	if (const ArraySym *hosts = theBench->serverSide()->hostsSym()) {
		if (hosts->count()) {
			addrs = &((ArraySym&)hosts->clone()->cast(strAddrArr));
			return String();
		}
	}
	return "no server_side.hosts specified";
}

String WebAxe4AsSym::proxies(ArraySym *&addrs) const {
	return addresses(theBench->proxySide(), addrs);
}
