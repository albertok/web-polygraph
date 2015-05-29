
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BEEP_BEEPCHANNEL_H
#define POLYGRAPH__BEEP_BEEPCHANNEL_H

class RawBeepMsg;

// maintains counter for BEEP channels
// mod arithmetic is not yet supported
class BeepChannel {
	public:
		BeepChannel(int anId = -1);

		int id() const { return theId; }

		int nextMsgNo();
		int nextSeqNo();

		void addedMsg(const RawBeepMsg &msg);
		bool consumedMsg(const RawBeepMsg &msg);

	protected:
		int theId;
		int theNextMsgNo;
		int theNextSeqNo;
		int theExpMsgNo;  // expected
		int theExpSeqNo;
};

#endif
