
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/OLog.h"
#include "base/ILog.h"
#include "base/XactStatRec.h"

XactStatRec::XactStatRec(): theOwnerSeqId(-1), theConnSeqId(-1), 
	theQueueMsec(0), theLifeMsec(0), theEmbedCount(-1) {
}

void XactStatRec::store(OLog &ol) const {
	ol << theId << theOwnerSeqId << theConnSeqId
		<< theStartTime << theQueueMsec << theLifeMsec
		<< theEmbedCount << theRepSize << theOid;
}

void XactStatRec::load(ILog &il) {
	il >> theId >> theOwnerSeqId >> theConnSeqId
		>> theStartTime >> theQueueMsec >> theLifeMsec
		>> theEmbedCount >> theRepSize >> theOid;
}
