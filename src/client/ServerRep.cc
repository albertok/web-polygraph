
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/OLog.h"
#include "base/ILog.h"
#include "runtime/httpHdrs.h"
#include "client/ServerRep.h"


ServerRep::ServerRep(const NetAddr &anAddr, int aHostIdx): 
	theAddr(anAddr), theHostIdx(aHostIdx), theSrvIdx(-1),
	theReqCount(0), theRespCount(0) {
}

void ServerRep::noteRequest() {
	theReqCount++;
}

void ServerRep::noteFirstHandResponse() {
	theRespCount++;
}

void ServerRep::store(OLog &log) const {
	log
		<< theAddr
		<< theReqCount
		<< theRespCount
		;
}

void ServerRep::load(ILog &log) {
	log
		>> theAddr
		>> theReqCount
		>> theRespCount
		;
}
