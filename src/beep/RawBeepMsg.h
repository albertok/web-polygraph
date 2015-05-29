
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BEEP_RAWBEEPMSG_H
#define POLYGRAPH__BEEP_RAWBEEPMSG_H

#include "xstd/String.h"

class RawBeepMsg {
	public:
		enum Types { bmtNone = 0, bmtMsg, bmtRpy, bmtAns, bmtErr, bmtNul, bmtEnd };

	public:
		RawBeepMsg(Types aType = bmtNone);

		int type() const { return theType; }
		int channel() const { return theChannel; }
		int no() const { return theNo; }
		int seqNo() const { return theSeqNo; }
		int ansNo() const { return theAnsNo; }
		const String &image() const { return theImage; }

		void type(int aType) { theType = aType; }
		void channel(int aChannel) { theChannel = aChannel; }
		void no(int aNo) { theNo = aNo; }
		void seqNo(int aNo) { theSeqNo = aNo; }
		void ansNo(int aNo) { theAnsNo = aNo; }
		void image(const String &anImage) { theImage = anImage; }

		const String &typeStr() const;
		void replyTo(const RawBeepMsg &msg); // form a reply

	protected:
		int theType;
		int theChannel;
		int theNo;
		int theSeqNo;
		int theAnsNo;
		String theImage;
};

#endif
