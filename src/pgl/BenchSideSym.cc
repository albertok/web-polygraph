
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglRateSym.h"
#include "pgl/BenchSideSym.h"
#include "pgl/BenchSym.h"



String BenchSideSym::TheType = "BenchSide";

static String strAddrArr = "addr[]";
static String strAddr_mask = "addr_mask";
static String strAddr_space = "addr_space";
static String strHosts = "hosts";
static String strMax_agent_load = "max_agent_load";
static String strMax_host_load = "max_host_load";
static String strAddresses = "addresses";
static String strIntArr = "int[]";
static String strCpuCores = "cpu_cores";


BenchSideSym::BenchSideSym(): RecSym(TheType, new PglRec), theBench(0) {
	theRec->bAdd(strAddrArr,  strHosts, 0);
	theRec->bAdd(RateSym::TheType, strMax_host_load, 0);
	theRec->bAdd(RateSym::TheType, strMax_agent_load, 0);
	theRec->bAdd(strAddrArr, strAddr_space, 0);
	theRec->bAdd(NetAddrSym::TheType, strAddr_mask, 0);
	theRec->bAdd(strAddrArr,  strAddresses, 0);
	theRec->bAdd(strIntArr, strCpuCores, 0);
}

BenchSideSym::BenchSideSym(const String &aType, PglRec *aRec): RecSym(aType, aRec), theBench(0) {
}

bool BenchSideSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *BenchSideSym::dupe(const String &type) const {
	if (isA(type))
		return new BenchSideSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}


bool BenchSideSym::hosts(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strHosts, addrs);
}

bool BenchSideSym::cpuCoresArray(Array< Array<int> > &cpuCores) const {
	ArraySym *a = getArraySym(strCpuCores);
	if (!a)
		return false;

	Array< Array<IntSym*> > syms;
	if (!a->exportNestedArrays(syms)) {
		cerr << loc() << "incorrect format " << strCpuCores << ": \"" <<
			*a << "\"" << endl;
		exit(1);
	}

	for (int i = 0; i < syms.count(); ++i) {
		const int cnt = syms[i].count();
		Array<int> arrInt(cnt);
		for (int j = 0; j < cnt; ++j)
			arrInt.append(syms[i][j]->val());
		cpuCores.append(arrInt);
	}

	return true;
}

ArraySym *BenchSideSym::hostsSym() const {
	return getArraySym(strHosts);
}

const ArraySym *BenchSideSym::addrSpace() const {
	return getArraySym(strAddr_space);
}

NetAddr BenchSideSym::addrMask() const {
	return getNetAddr(strAddr_mask);
}

NetAddrSym* BenchSideSym::addrMaskSym() const {
	SynSymTblItem *mi = 0;
	Assert(theRec->find(strAddr_mask, mi));
	if (!mi->sym())
		return 0;
	return &(NetAddrSym&)mi->sym()->cast(NetAddrSym::TheType);
}

bool BenchSideSym::maxHostLoad(double &load) const {
	return getRate(strMax_host_load, load);
}

bool BenchSideSym::maxAgentLoad(double &load) const {
	return getRate(strMax_agent_load, load);
}

bool BenchSideSym::addresses(Array<NetAddrSym *> &addrs) const {
	if (ArraySym *a = getArraySym(strAddresses)) {
		for (int i = 0; i < a->count(); ++i) {
			NetAddrSym *const addr =
				&SymCast(NetAddrSym, *(*a)[i]->clone());
			addrs.append(addr);
		}
		return true;
	}
	return false;
}
