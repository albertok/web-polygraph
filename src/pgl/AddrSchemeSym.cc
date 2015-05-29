
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "pgl/PglClonerSym.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/PglNetAddrRangeSym.h"
#include "pgl/AddrSchemeSym.h"
#include "pgl/pglStrIs.h"



String AddrSchemeSym::TheType = "AddrScheme";

static String strKind = "kind";
static String strNet = "net";


AddrSchemeSym::AddrSchemeSym(const String &aType, PglRec *aRec): RecSym(aType, aRec), theBench(0) {
	theRec->bAdd(StringSym::TheType, strKind, 0);
}

bool AddrSchemeSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

String AddrSchemeSym::kind() const {
	return getString(strKind);
}

ArraySym *AddrSchemeSym::robots(const BenchSym *bench, String &err) const {
	setBench(bench);
	ArraySym *res = 0; 
	err = robots(res);
	setBench(0);
	return res;
}

ArraySym *AddrSchemeSym::servers(const BenchSym *bench, String &err) const {
	setBench(bench);
	ArraySym *res = 0; 
	err = servers(res);
	setBench(0);
	return res;
}

ArraySym *AddrSchemeSym::proxies(const BenchSym *bench, String &err) const {
	setBench(bench);
	ArraySym *res = 0; 
	err = proxies(res);
	setBench(0);
	return res;
}

String AddrSchemeSym::proxies(ArraySym *&) const {
	return "address scheme does not support proxy-side addresses";
}

void AddrSchemeSym::setBench(const BenchSym *bench) const {
	theBench = bench;
}

// find min subnet that can fit maxAddrPerHost addresses
String AddrSchemeSym::minSubnet(int maxAddrPerHost, int &minSubnet) const {
	minSubnet = 32;
	int maxAddrPerSnet = 1;
	while (minSubnet > 0 && maxAddrPerSnet < maxAddrPerHost) {
		maxAddrPerSnet *= 2;
		--minSubnet;
	}
	if (maxAddrPerSnet < maxAddrPerHost || !minSubnet)
		return "not enough IP addresses";
	return String();
}

// mask.x-x.y-y
String AddrSchemeSym::ipRangeToStr(const NetAddrSym &mask, int minX, int xCnt, int minY, int yCnt) const {
	char bufMask[256];
	ofixedstream osMask(bufMask, sizeof(bufMask));

	if (mask.ifName())
		osMask << mask.ifName() << "::";

	osMask << mask.val() << ends;

	// leave the strNet part alone
	char *dot1 = strchr(bufMask, '.');
	Assert(dot1);
	char *dot2 = strchr(dot1+1, '.');
	Assert(dot2);
	dot2++;

	ofixedstream os(dot2, sizeof(bufMask) - (dot2-bufMask));
	printOctetRange(os, minX, xCnt);
	os << '.';
	printOctetRange(os, minY, yCnt);

	if (mask.val().port() >= 0)
		os << ':' << mask.val().port();

	int subnet;
	if (mask.subnet(subnet))
		os << '/' << subnet;

	os << ends;

	return bufMask;
}

void AddrSchemeSym::printOctetRange(ostream &os, int first, int count) const {
	const int last = first + count -1;
	if (first == last)
		os << first;
	else
		os << first << '-' << last;
}

void AddrSchemeSym::makeAddrSym(const NetAddrSym &mask, int x, int y, int subnet, NetAddrSym &nas) const {
	String str = ipRangeToStr(mask, x, 1, y, 1);
	NetAddr addr;
	Assert(pglIsNetAddr(str, addr));
	nas.val(addr);

	int newSnet = subnet;
	if (mask.subnet(newSnet)) {
		if (subnet > 0 && subnet != newSnet) {
			clog << mask.loc() << "warning: subnet in side's addr_mask (/" 
				<< newSnet << ") differs from computed subnet (/" 
				<< subnet << "); using /" << newSnet << endl;
		}
	}

	if (newSnet > 0) {
		nas.setSubnet(newSnet);
	}
}

// find minimum int X such that X >= n and X is divisible by factor
int AddrSchemeSym::singleDiv(int factor, double n) const {
	return (int)(factor * xceil(n, factor));
}

// find minimum int X such that X >= n/d and X is divisible by factor
// in other words,
// divide n into X groups, no more than d elements each,
// and make sure that X is divisible by factor
// return X
int AddrSchemeSym::doubleDiv(int factor, double n, double d) const {
	const double apx = xceil(n, d);
	return singleDiv(factor, apx);
}

void AddrSchemeSym::kind(const String &aKind) {
	SynSymTblItem *ki = 0;
	Assert(theRec->find(strKind, ki));
	Assert(!ki->sym());
	ki->sym(new StringSym(aKind));
}

void AddrSchemeSym::addAddrToRanges(Array<PglNetAddrRange*> &ranges, const NetAddrSym &addr) {
	PglNetAddrRange r;
	ostringstream buf; // XXX: expensive mallocs
	addr.printUnquoted(buf);
	buf << ends;
	Assert(r.parse(buf.str().c_str()));
	streamFreeze(buf, false);

	// check if can merge r with last range, add new range if failed
	if (!ranges.count() || !ranges.last()->canMerge(r))
		ranges.append(new PglNetAddrRange);
	Assert(ranges.count());
	ranges.last()->merge(r);
}

// merge ranges if possible, merged ranges are deleted and set to 0
void AddrSchemeSym::mergeRanges(Array<PglNetAddrRange*> &ranges) {
	bool merged = true;
	do {
		merged = false;
		for (int left = 0, right = 1; !merged && right < ranges.count(); ++right) {
			PglNetAddrRange *&r = ranges[right];
			if (!r) // deleted by earlier merges
				continue;
			if (ranges[left] && ranges[left]->canMerge(*r)) {
				ranges[left]->merge(*r);
				delete r;
				r = 0;
				merged = true;
			} else {
				left = right;
			}
		}
	} while (merged);
}

ArraySym *AddrSchemeSym::rangesToAddrs(const Array<PglNetAddrRange*> &ranges, const int addrLoad) {
	// convert an array of ranges into ArraySym
	ArraySym *addrs = new ArraySym(NetAddrSym::TheType);
	{for (int i = 0; i < ranges.count(); ++i) {
		if (!ranges[i])
			continue;
		NetAddrRangeSym rs;
		rs.range(ranges[i]);
		addrs->add(rs);
	}}

	if (addrLoad > 1) {
		// addrLoad agents for each address (clone addrs addrLoad times)
		ArraySym *res = new ArraySym(NetAddrSym::TheType);
		res->add(ClonerSym(*addrs, addrLoad));
		delete addrs;
		addrs = res;
	}

	return addrs;
}
