
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_SERVERREP_H
#define POLYGRAPH__CLIENT_SERVERREP_H

// client side "representative" of an origin server 
// holds client side knowledge about a server

#include "xstd/NetAddr.h"
#include "base/UniqId.h"

class OLog;
class ObjId;
class RepHdr;

class ServerRep {
	public:
		ServerRep(const NetAddr &anAddr, int hostIdx);

		const NetAddr &addr() const { return theAddr; }
		int hostIdx() const { return theHostIdx; }
		int serverIdx() const { return theSrvIdx; }

		int requests() const { return theReqCount; }
		int responses() const { return theRespCount; }

		void serverIdx(int aSrvIdx) { theSrvIdx = aSrvIdx; }

		void store(OLog &log) const;
		void load(ILog &log);

		void noteRequest();
		void noteFirstHandResponse();

	protected:
		NetAddr theAddr;

		int theHostIdx;    // index into the HostMap
		int theSrvIdx;     // index into ServerRep map on Client

		int theReqCount;   // requests to this server
		int theRespCount;   // first-hand responses from this server
};

#endif
