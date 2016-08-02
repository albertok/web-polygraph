
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/sstream.h"

#include "xstd/RegEx.h"
#include "xstd/NetAddr.h"
#include "base/polyLogCats.h"
#include "runtime/AddrMap.h"
#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"
#include "client/MembershipMap.h"
#include "client/RegExGroups.h"


/* RegExGrpOne */

RegExGrpOne::RegExGrpOne(RegEx *aVal): theVal(aVal) {
}

bool RegExGrpOne::match(const RegExMatchee &m, int flags, int *errNo) const {
	Assert(theVal);
	if (theVal->image().cmp("user_group=", 11) == 0)
		return matchGroups(m, flags, errNo);
	else
		return matchUrl(m, flags, errNo);
}

bool RegExGrpOne::matchUrl(const RegExMatchee &m, int flags, int *errNo) const {
	const char *str = 0; // optimize: cache the lookup
	if (theVal->image().cmp("url=", 4) == 0)
		str = m.url;
	else
	if (theVal->image().cmp("url_host=", 9) == 0)
		str = m.urlHost;
	else
	if (theVal->image().cmp("url_host_ip=", 12) == 0)
		str = hostIps(m.urlHost);
	else
	if (theVal->image().cmp("url_path=", 9) == 0)
		str = m.urlPath;

	if (!Should(str))
		str = m.url;

	return theVal->match(str, flags, errNo);
}

// there is a match if any of the groups matches
bool RegExGrpOne::matchGroups(const RegExMatchee &m, int flags, int *errNo) const {
	int err;
	if (!errNo)
		errNo = &err;
	*errNo = 0;
	Assert(m.memberships);
	const Array<MembershipMap*> &ms = *m.memberships;
	for (int i = 0; !*errNo && i < ms.count(); ++i) {
		if (ms[i]->match(theVal, m.userName, flags, errNo))
			return true;
	}
	return false;
}

const char *RegExGrpOne::hostIps(const char *hostName) const {
	Assert(TheAddrMap);
	const NetAddr host(hostName, -1);

	if (!host.isDomainName()) // already an IP address
		return hostName;

	if (!TheAddrMap->has(host)) {
		if (ReportError2(errAclHostIpLookup, lgcCltSide))
			Comment << "address maps cannot resolve: " << hostName << endc;
		return hostName;
	}
		
	static ostringstream os;
	streamFreeze(os, false);
	os.seekp(0);
	for (AddrMapAddrIter i = TheAddrMap->addrIter(host); i; ++i) {
		if (os.tellp())
			os << ',';
		os << i.addr().addrA();
	}
	os << ends;
	return os.str().c_str();
}

ostream &RegExGrpOne::print(ostream &os) const {
	if (theVal)
		return theVal->print(os);
	else
		return os << "<none>";
}


/* RegExGrpNot */

RegExGrpNot::RegExGrpNot(RegExGroup *aVal): theVal(aVal) {
}

bool RegExGrpNot::match(const RegExMatchee &m, int flags, int *errNo) const {
	int err = 0;
	if (!errNo)
		errNo = &err;
	const bool res = theVal->match(m, flags, errNo);
	return (*errNo) ? false : !res;
}

ostream &RegExGrpNot::print(ostream &os) const {
	if (theVal)
		return theVal->print(os << "!(") << ')';
	else
		return os << "<none>";
}


/* RegExGrpOper */

void RegExGrpOper::add(RegExGroup *item) {
	theItems.append(item);
}

ostream &RegExGrpOper::print(ostream &os) const {
	if (!theItems.count()) {
		os << "<none>";
	} else
	if (theItems.count() == 1) {
		theItems.last()->print(os);
	} else {
		os << '(';
		for (int i = 0; i < theItems.count(); ++i) {
			if (i)
				os << ' ' << operImage() << ' ';
			theItems[i]->print(os);
		}
		os << ')';
	}
	return os;
}


/* RegExGrpAnd */

bool RegExGrpAnd::match(const RegExMatchee &m, int flags, int *) const {
	for (int i = 0; i < theItems.count(); ++i) {
		if (!theItems[i]->match(m, flags))
			return false;
	}
	return true; // zero &&s is true (just like zero *s is 1)
}

const char *RegExGrpAnd::operImage() const {
	return "&&";
}


/*  RegExGrpOr */

bool RegExGrpOr::match(const RegExMatchee &m, int flags, int *) const {
	for (int i = 0; i < theItems.count(); ++i) {
		if (theItems[i]->match(m, flags))
			return true;
	}
	return false; // zero ||s is false (just like zero +s is 0)
}

const char *RegExGrpOr::operImage() const {
	return "||";
}

