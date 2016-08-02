
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "xstd/TblDistr.h"
#include "pgl/PglRec.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/HostsBasedSym.h"
#include "pgl/SslWrapSym.h"


String HostsBasedSym::TheType = "HostsBased";

static String strSslWrapArr = "SslWrap[]";
static String strAddrArr = "addr[]";
static String strAddresses = "addresses";
static String strHosts = "hosts";
static String strSslWraps = "ssl_wraps";


HostsBasedSym::HostsBasedSym(const String &aType): RecSym(aType, new PglRec) {
	// originally, strAddresses were strHosts, and we did not want to
	// change class name when the field name has changed 
	// to avoid renaming a bunhc of files
	theRec->bAdd(strAddrArr, strAddresses, 0);
	theRec->bAdd(strSslWrapArr, strSslWraps, 0);
}

HostsBasedSym::HostsBasedSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool HostsBasedSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *HostsBasedSym::dupe(const String &type) const {
	if (isA(type))
		return new HostsBasedSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

int HostsBasedSym::hostCount() const {
	if (ArraySym *a = getArraySym(strAddresses))
		return a->count();
	else
		return 0;
}

NetAddrSym *HostsBasedSym::host(int idx) const {
	if (ArraySym *a = getArraySym(strAddresses)) {
		Assert(idx < a->count());
		return &(NetAddrSym&)(*a)[idx]->cast(NetAddrSym::TheType);
	}
	return 0;
}

const ArraySym *HostsBasedSym::addresses() const {
	return getArraySym(strAddresses);
}

bool HostsBasedSym::addresses(ArraySym &addrs) const {
	if (const ArraySym *a = addresses()) {
		addrs.append(*a);
		return true;
	}
	return false;
}

bool HostsBasedSym::sslWraps(Array<SslWrapSym*> &syms, RndDistr *&sel) const {
	SynSymTblItem *wi = 0;
	Assert(theRec->find(strSslWraps, wi));
	if (!wi->sym())
		return false; // undefined

	ArraySym &a = (ArraySym&)wi->sym()->cast(ArraySym::TheType);
	a.exportA(syms);
	Array<double> probs;
	a.copyProbs(probs);
	sel = TblDistr::FromDistrTable(type() + "-" + strSslWraps, probs);
	return true;
}
