
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglRec.h"
#include "pgl/PglRecSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglNetAddrSym.h"



String NetAddrSym::TheType = "addr";

static String strHost = "host";
static String strIf_name = "if_name";
static String strPort = "port";
static String strSubnet = "subnet";


NetAddrSym::NetAddrSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(StringSym::TheType, strIf_name, 0);
	theRec->bAdd(StringSym::TheType, strHost, 0);
	theRec->bAdd(IntSym::TheType, strPort, 0);
	theRec->bAdd(IntSym::TheType, strSubnet, 0);
}

NetAddrSym::NetAddrSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool NetAddrSym::isA(const String &type) const {
	return type == TheType || RecSym::isA(type);
}

SynSym *NetAddrSym::dupe(const String &type) const {
	if (isA(type))
		return new NetAddrSym(this->type(), theRec->clone());
	return ExpressionSym::dupe(type);
}

String NetAddrSym::ifName() const {
	return getString(strIf_name);
}

bool NetAddrSym::subnet(int &sn) const {
	return getInt(strSubnet, sn);
}

NetAddr NetAddrSym::val() const {
	Assert(theRec);
	SynSymTblItem *host = 0;
	SynSymTblItem *port = 0;
	Assert(theRec->find(strHost, host) && theRec->find(strPort, port));
	return NetAddr(
		host->sym() ? 
			((const StringSym&)host->sym()->cast(StringSym::TheType)).val() : String(),
		port->sym() ?
			((const IntSym&)port->sym()->cast(IntSym::TheType)).val() : -1);
}

void NetAddrSym::val(const NetAddr &addr) {
	Assert(theRec);
	SynSymTblItem *host = 0;
	Assert(theRec->find(strHost, host));
	delete host->sym();
	host->sym(addr.addrA().len() ? new StringSym(addr.addrA()) : 0);
	portVal(addr.port());
}

void NetAddrSym::portVal(int newPort) {
	Assert(theRec);
	SynSymTblItem *port = 0;
	Assert(theRec->find(strPort, port));
	delete port->sym();
	port->sym(newPort >= 0 ? new IntSym(newPort) : 0);
}

void NetAddrSym::setIfname(const String &aName) {
	SynSymTblItem *ifnamei = 0;
	Assert(theRec->find(strIf_name, ifnamei));
	delete ifnamei->sym();
	ifnamei->sym(new StringSym(aName));
}

void NetAddrSym::setSubnet(int aSubnet) {
	SynSymTblItem *subneti = 0;
	Assert(theRec->find(strSubnet, subneti));
	delete subneti->sym();
	subneti->sym(new IntSym(aSubnet));
}

void NetAddrSym::printUnquoted(ostream &os) const {
	if (ifName())
		os << ifName() << "::";

	// the address itself
	os << val();

	int snet = -1;
	if (subnet(snet))
		os << '/' << snet;
}

ostream &NetAddrSym::print(ostream &os, const String &) const {
	os << '\'';
	printUnquoted(os);
	return os << '\'';
}
