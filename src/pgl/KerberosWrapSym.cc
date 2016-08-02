
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/KerberosWrapSym.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglTimeSym.h"


const String KerberosWrapSym::TheType = "KerberosWrap";

static String strAddrArr = "addr[]";
static String strRealm = "realm";
static String strServers = "servers";
static String strServersTcp = "servers_tcp";
static String strServersUdp = "servers_udp";
static String strTimeout = "timeout";


KerberosWrapSym::KerberosWrapSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(StringSym::TheType, strRealm, 0);
	theRec->bAdd(strAddrArr, strServers, 0);
	theRec->bAdd(strAddrArr, strServersTcp, 0);
	theRec->bAdd(strAddrArr, strServersUdp, 0);
	theRec->bAdd(TimeSym::TheType, strTimeout, 0);
}

KerberosWrapSym::KerberosWrapSym(const String &aType, PglRec *aRec):
	RecSym(aType, aRec) {
}

bool KerberosWrapSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *KerberosWrapSym::dupe(const String &type) const {
	if (isA(type))
		return new KerberosWrapSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

String KerberosWrapSym::realm() const {
	return getString(strRealm);
}

bool KerberosWrapSym::servers(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strServers, addrs);
}

bool KerberosWrapSym::serversTcp(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strServersTcp, addrs);
}

bool KerberosWrapSym::serversUdp(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strServersUdp, addrs);
}

Time KerberosWrapSym::timeout() const {
	return getTime(strTimeout);
}
