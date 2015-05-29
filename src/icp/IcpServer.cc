
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/polyLogCats.h"
#include "cache/Cache.h"
#include "icp/IcpMsg.h"
#include "icp/IcpServer.h"


IcpServer::IcpServer(): theCache(0) {
}

void IcpServer::cache(Cache *aCache) {
	Assert(!theCache);
	theCache = aCache;
}

void IcpServer::noteRequest(const IcpMsg &m) {
	IcpMsg r(m); // is it ok to reuse all parts?
	if (theCache)
		r.opCode(theCache->cached(m.oid()) ? icpHit : icpMissNoFetch);
	else
		r.opCode(icpDenied);
	r.send(theSock);
}

int IcpServer::logCat() const {
	return lgcSrvSide;
}
