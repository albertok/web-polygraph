
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/CacheSym.h"
#include "pgl/RobotSym.h"
#include "pgl/ServerSym.h"
#include "pgl/ProxySym.h"



const String ProxySym::TheType = "Proxy";

static String strCache = "cache";
static String strClient = "client";
static String strServer = "server";


ProxySym::ProxySym(): AgentSym(TheType) {
	theRec->bAdd(RobotSym::TheType, strClient, 0);
	theRec->bAdd(ServerSym::TheType, strServer, new ServerSym);
	theRec->bAdd(CacheSym::TheType, strCache, new CacheSym);
}

ProxySym::ProxySym(const String &aType, PglRec *aRec): AgentSym(aType, aRec) {
}

bool ProxySym::isA(const String &type) const {
	return type == TheType || AgentSym::isA(type);
}

SynSym *ProxySym::dupe(const String &type) const {
	if (isA(type))
		return new ProxySym(this->type(), theRec->clone());
	return AgentSym::dupe(type);
}

RobotSym *ProxySym::client() const {
	SynSymTblItem *ri = 0;
	Assert(theRec->find(strClient, ri));
	if (ri->sym())
		return &(RobotSym&)ri->sym()->cast(RobotSym::TheType);
	return 0;
}

ServerSym *ProxySym::server() const {
	SynSymTblItem *si = 0;
	Assert(theRec->find(strServer, si));
	if (si->sym())
		return &(ServerSym&)si->sym()->cast(ServerSym::TheType);
	return 0;
}

AgentSym *ProxySym::side(const String &sideType) const {
	if (sideType == RobotSym::TheType)
		return client();
	if (sideType == ServerSym::TheType)
		return server();
	return 0;
}

CacheSym *ProxySym::cache() const {
	SynSymTblItem *ci = 0;
	Assert(theRec->find(strCache, ci));
	if (ci->sym())
		return &(CacheSym&)ci->sym()->cast(CacheSym::TheType);
	cerr << loc() << "must configure cache for each proxy" << endl << xexit;
	return 0;
}

String ProxySym::msgTypesField() const {
	Assert(false); // should not be called
	return String();
}
