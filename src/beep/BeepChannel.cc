
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "beep/RawBeepMsg.h"
#include "beep/BeepChannel.h"


BeepChannel::BeepChannel(int anId): theId(anId),
	theNextMsgNo(0), theNextSeqNo(0), theExpMsgNo(-1), theExpSeqNo(-1) {
}

int BeepChannel::nextMsgNo() {
	return theNextMsgNo++;
}

int BeepChannel::nextSeqNo() {
	return theNextSeqNo;
}

void BeepChannel::addedMsg(const RawBeepMsg &msg) {
	theNextSeqNo += msg.image().len();
}

bool BeepChannel::consumedMsg(const RawBeepMsg &msg) {
	if (theExpMsgNo < 0)
		theExpMsgNo = msg.no();
	else
	if (msg.no() != theExpMsgNo)
		return false;
	theExpMsgNo++;

	if (theExpSeqNo < 0)
		theExpSeqNo = msg.seqNo();

	if (msg.seqNo() != theExpSeqNo)
		return false;
	theExpSeqNo += msg.image().len();

	return true;
}
