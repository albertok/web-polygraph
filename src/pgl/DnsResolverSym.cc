
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/DnsResolverSym.h"
#include "pgl/DnsCacheSym.h"



String DnsResolverSym::TheType = "DnsResolver";

static String strAddrArr = "addr[]";
static String strServers = "servers";
static String strTimeout = "timeout";
static String strQueryType = "query_type";
static String strDnsCache = "cache";


DnsResolverSym::DnsResolverSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(strAddrArr, strServers, 0);
	theRec->bAdd(TimeSym::TheType, strTimeout, 0);
	theRec->bAdd(StringSym::TheType, strQueryType, 0);
	theRec->bAdd(DnsCacheSym::TheType, strDnsCache, new DnsCacheSym);
}

DnsResolverSym::DnsResolverSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool DnsResolverSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *DnsResolverSym::dupe(const String &type) const {
	if (isA(type))
		return new DnsResolverSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

Time DnsResolverSym::timeout() const {
	return getTime(strTimeout);
}

bool DnsResolverSym::servers(Array<NetAddr*> &srvs) const {
	return getNetAddrs(strServers, srvs);
}

String DnsResolverSym::queryType() const {
	return getString(strQueryType);

}

DnsCacheSym *DnsResolverSym::cache() const {
	SynSymTblItem *ci = 0;
	Assert(theRec->find(strDnsCache, ci));
	if (ci->sym())
		return &(DnsCacheSym&)ci->sym()->cast(DnsCacheSym::TheType);
	return 0;
}

