
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "beep/RawBeepMsg.h"

RawBeepMsg::RawBeepMsg(Types aType): theType(aType), 
	theChannel(-1), theNo(-1), theSeqNo(-1), theAnsNo(-1) {
}

void RawBeepMsg::replyTo(const RawBeepMsg &msg) {
	channel(msg.channel());
}

const String &RawBeepMsg::typeStr() const {
	static const String TypeStrs[bmtEnd] = 
		{ 0, "MSG", "RPY", "ANS", "ERR", "NUL" };

	if (bmtNone < theType && theType < bmtEnd)
		return TypeStrs[theType];
	else
		return TypeStrs[bmtNone];
}
