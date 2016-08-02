
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "xstd/NetAddr.h"
#include "xstd/gadgets.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglNetAddrParts.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/pglStrIs.h"


PglNetAddrRange::PglNetAddrRange(): theSubnet(-1) {
}

void PglNetAddrRange::reset() {
	PglStrRange::reset();
	theAddr = String();
	theIfName = String();
	theSubnet = -1;
}

void PglNetAddrRange::netmask(InAddress &netm) const {
	String str;
	startIter();
	currentIter(str);
	NetAddr firstNetAddr;
	Should(pglIsNetAddr(str, firstNetAddr));

	int subnet = theSubnet;
	if (subnet < 0) {
		// default subnet length is the size of the address: 32 or 128
		subnet = firstNetAddr.addrN().len() * 8;
	}
	netm = InAddress::NetMask(firstNetAddr.addrN().family(), subnet);
}

ArraySym *PglNetAddrRange::toSyms(const TokenLoc &loc) const {
	ArraySym *arr = new ArraySym(NetAddrSym::TheType);

	startIter();
	do {
		NetAddrSym nas;
		currentAddrSym(nas);
		if (loc)
			nas.loc(loc);
		arr->add(nas); // XXX: replace with add(this)

	} while (nextIter());

	return arr;
}

void PglNetAddrRange::toAddrs(Array<NetAddr*> &addrs) const {
	startIter();
	do {
		NetAddr addr;
		currentNetAddr(addr);

		addrs.append(new NetAddr(addr));
	} while (nextIter());
}

void PglNetAddrRange::toAddrs(AddrIter iter) const {
	startIter();
	do {
		NetAddr addr;
		currentNetAddr(addr);

		iter(addr);
	} while (nextIter());
}

void PglNetAddrRange::addrAt(int idx, NetAddrSym &nas) const {
	startIter();
	skipIter(idx);
	currentAddrSym(nas);
}

void PglNetAddrRange::currentAddrSym(NetAddrSym &nas) const {
	NetAddr addr;
	currentNetAddr(addr);

	nas.val(addr);
	if (theIfName)
		nas.setIfname(theIfName);
	if (theSubnet >= 0)
		nas.setSubnet(theSubnet);
}

void PglNetAddrRange::currentNetAddr(NetAddr &addr) const {
	String str;
	currentIter(str);
	if (!pglIsNetAddr(str, addr))
		cerr << here << "error: invalid address " << str
			<< " extracted from a malformed address range " << toStr() << endl << xexit;
}

bool PglNetAddrRange::parse(const String &val) {
	PglNetAddrParts parts(val);

	if (parts.error()) {
		cerr << "error: failed to parse '" << val << "' address range: " <<
			parts.error() << endl;
		return false;
	}

	theIfName = parts.ifName();
	theSubnet = parts.subnet();
	// stored together but parsed separately because bases may differ
	theAddr = parts.hosts() + parts.ports();

	// IPv6 requires hexidecimal base and needs '[]' escapes handled separately
	if (parts.hostKind() == PglNetAddrParts::hkIPv6) {
		static const String openb = "[";
		static const String closeb = "]";
		addRangePoint(openb);
		currentBase(16);
		// strip '[]'
		const String bareHosts = parts.hosts()(1, parts.hosts().len()-1);
		if (!PglStrRange::parse(bareHosts))
			return false;
		addRangePoint(closeb);
	} else {
		currentBase(10);
		if (!PglStrRange::parse(parts.hosts()))
			return false;
	}

	if (parts.ports().len() > 0) {
		currentBase(10); // ports are always decimal
		return PglStrRange::parse(parts.ports());
	} else {
		return true;
	}
}

bool PglNetAddrRange::canMergeSameType(const StringArrayBlock &b) const {
	const PglNetAddrRange &r = (const PglNetAddrRange &)b;

	if (!count()) // brand new object
		return true;

	if ((theIfName || r.theIfName) && theIfName != r.theIfName)
		return false;

	if (theSubnet != r.theSubnet)
		return false;

	return PglStrRange::canMergeSameType(r);
}

void PglNetAddrRange::mergeSameType(const StringArrayBlock &b) {
	const PglNetAddrRange &r = (const PglNetAddrRange &)b;
	theIfName = r.theIfName;
	theSubnet = r.theSubnet;
	PglStrRange::mergeSameType(r);
}

bool PglNetAddrRange::optRangeBeg(char ch) const {
	return ch == '.' || ch == ':';
}

ostream &PglNetAddrRange::print(ostream &os) const {
	if (theIfName)
		os << theIfName << "::";
	
	PglStrRange::print(os);

	if (theSubnet >= 0)
		os << '/' << theSubnet;

	return os;
}
