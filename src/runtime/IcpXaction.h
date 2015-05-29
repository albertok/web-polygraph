
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_ICPXACTION_H
#define POLYGRAPH__RUNTIME_ICPXACTION_H

#include "xstd/NetAddr.h"
#include "base/ObjId.h"
#include "icp/IcpOpCode.h"

// base class for ICP client and server xactions
class IcpXaction {
	public:
		IcpXaction() { reset(); }

		void reset();

		int logCat() const { return theLogCat; }
		const ObjId &oid() const { return theOid; }
		Time lifeTime() const { return theLifeTime; }
		Size repSize() const { return theRepSize; }

		bool hit() const { return theResult == icpHit; }
		bool timedout() const { return theResult == icpTimeout; }

	protected:
		void start();
		void finish(Error err);

	protected:
		NetAddr theAddr;
		ObjId theOid;       // cached from theReason

		Time theStartTime;  // started doing something
		Time theLifeTime;   // [start, stop];

		Size theRepSize;

		IcpOpCode theResult;

		int theLogCat;
};

#endif
