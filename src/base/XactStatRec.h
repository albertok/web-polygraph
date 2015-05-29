
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_XACTSTATREC_H
#define POLYGRAPH__BASE_XACTSTATREC_H

#include "base/ObjId.h"

// summarizes single transaction statistics

class XactStatRec {
	public:
		XactStatRec();		

		Time startTime() const { return theStartTime; }
		Time queueTime() const { return Time::Msec(theQueueMsec); }
		Time lifeTime() const { return Time::Msec(theLifeMsec); }

		inline void times(Time start, Time queue, Time life);

		void store(OLog &ol) const;
		void load(ILog &il);

	public:
		UniqId theId;
		int theOwnerSeqId; // xaction owner
		int theConnSeqId;     // connection id

		Time theStartTime; // started
		int theQueueMsec;  // time spent in a queue (msec)
		int theLifeMsec;   // (msec)

		int theEmbedCount;  // number of embedded object in the reply
		Size theRepSize;    // reply size
		ObjId theOid;   // various details about the transfered object
};

/* inlined methods */

inline
void XactStatRec::times(Time start, Time queue, Time life) {
	theStartTime = start;
	theQueueMsec = queue >= 0 ? (start - queue).msec() : -1;
	theLifeMsec = life.msec();
}

#endif
