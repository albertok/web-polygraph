
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "beep/RawBeepMsg.h"
#include "beep/BeepChannel.h"
#include "beep/BeepSessionMgr.h"

static const String TheBeepFrameTrailer = "END";


BeepSessionMgr::BeepSessionMgr(int anId):
	theInBuf(160*1024), theOutBuf(160*1024), theId(anId),
	needMoreContent(false), needMoreSpace(false) {
}

BeepSessionMgr::~BeepSessionMgr() {
	while (theChannels.count())
		delete theChannels.pop();
}

int BeepSessionMgr::channelIdAt(int idx) const {
	return theChannels[idx]->id();
}

void BeepSessionMgr::startChannel(int anId, const String &) {
	BeepChannel *ch = new BeepChannel(anId /*, profile*/);
	theChannels.append(ch);
}

bool BeepSessionMgr::putMsg(const Msg &msg) {
	Assert(msg.channel() >= 0);
	Assert(msg.type() > 0);

	if (!goodOut())
		return false;

	BeepChannel *ch = findChannel(msg.channel());
	Assert(ch);
	
	IOBufState bufState;
	theOutBuf.saveState(bufState);

	/* frame header */

	putStr(msg.typeStr());
	putSpace();
	putInt(msg.channel());
	putSpace();
	putInt(ch->nextMsgNo());
	putSpace();
	putChar('.');
	putSpace();
	putInt(ch->nextSeqNo());
	putSpace();
	putInt(msg.image().len());

	if (msg.type() == Msg::bmtAns) {
		putSpace();
		putInt(msg.ansNo());
	}

	putChar('\r'); putChar('\n');

	// payload
	putStr(msg.image());

	// trailer
	putStr(TheBeepFrameTrailer);
	putChar('\r'); putChar('\n');

	if (goodOut()) {
		ch->addedMsg(msg);
		return true;
	} else {
		theOutBuf.restoreState(bufState);
		return false;
	}
}

bool BeepSessionMgr::getMsg(Msg &msg) {
	if (!goodIn())
		return false;

	IOBufState bufState;
	theInBuf.saveState(bufState);

	msg.type(Msg::bmtNone);
	if (skipStrIfMatch("MSG", 3))
		msg.type(Msg::bmtMsg);
	else
	if (skipStrIfMatch("RPY", 3))
		msg.type(Msg::bmtRpy);
	else
	if (skipStrIfMatch("ANS", 3))
		msg.type(Msg::bmtAns);
	else
	if (skipStrIfMatch("ERR", 3))
		msg.type(Msg::bmtErr);
	else
	if (skipStrIfMatch("NUL", 3))
		msg.type(Msg::bmtNul);
	else 
	if (goodIn()) {
		error("malformed message header");
		return false;
	}

	skipSpace();

	msg.channel(getInt());
	skipSpace();
	msg.no(getInt());
	skipSpace();
	skipChar('.');
	skipSpace();
	msg.seqNo(getInt());
	skipSpace();
	const int size = getInt();

	if (msg.type() == Msg::bmtAns) {
		skipSpace();
		msg.ansNo(getInt());
	}

	skipCrLf();

	if (size >= 0)
		msg.image(getStr(size));

	if (skipTrailer()) {
		Channel *ch = findChannel(msg.channel());
		if (!ch)
			error("message on a nonexistant channel");

		if (goodIn()) {
			if (!ch->consumedMsg(msg))
				error("wrong input sequencing number(s)");
			else
				theInBuf.pack(); // incomatible with theInBuf.restoreState
		}
	}

	if (needMoreContent)
		theInBuf.restoreState(bufState);

	return goodIn();
}

bool BeepSessionMgr::hasSpaceIn() const {
	// keep it half-full?
	return theInBuf.spaceSize() > theInBuf.contSize();
}

bool BeepSessionMgr::hasSpaceOut() const {
	return !needMoreSpace;
}

char *BeepSessionMgr::spaceIn(Size &size) {
	size = theInBuf.spaceSize();
	return theInBuf.space();
}

void BeepSessionMgr::spaceInUsed(Size size) {
	theInBuf.appended(size);
	needMoreContent = false;
}

bool BeepSessionMgr::hasContentIn() const {
	return theInBuf.contSize() > 0;
}

bool BeepSessionMgr::hasContentOut() const {
	return theOutBuf.contSize() > 0;
}

const char *BeepSessionMgr::contentOut(Size &size) const {
	size = theOutBuf.contSize();
	return theOutBuf.content();
}

void BeepSessionMgr::contentOutUsed(Size size) {
	theOutBuf.consumed(size);
	theOutBuf.pack();
	needMoreSpace = false;
}

BeepChannel *BeepSessionMgr::findChannel(int chId) {
	for (int i = 0; i < theChannels.count(); ++i) {
		if (theChannels[i]->id() == chId)
			return theChannels[i];
	}
	return 0;
}

bool BeepSessionMgr::putStr(const String &s) {
	if (goodOut()) {
		if (theOutBuf.spaceSize() < s.len())
			errorNoSpace();
		else
			theOutBuf.append(s.data(), s.len());
	}
	return goodOut();
}

bool BeepSessionMgr::putSpace() {
	return putChar(' ');
}

bool BeepSessionMgr::putChar(char ch) {
	if (goodOut()) {
		if (theOutBuf.spaceSize() < 1)
			errorNoSpace();
		else
			theOutBuf.append(&ch, 1);
	}
	return goodOut();
}

bool BeepSessionMgr::putInt(int n) {
	if (goodOut()) {
		if (theOutBuf.spaceSize() > 0) {
			ofixedstream os(theOutBuf.space(), theOutBuf.spaceSize());
			if (os << n)
				theOutBuf.appended(Size(os.tellp()));
			else
				errorNoSpace();
		} else {
			errorNoSpace();
		}
	}
	return goodOut();
}

bool BeepSessionMgr::skipChar(char ch) {
	if (goodIn()) {
		if (theInBuf.contSize() < 1)
			errorNoContent();
		else
		if (*theInBuf.content() == ch)
			theInBuf.consumed(1);
		else
			error("malformed incoming data");
	}
	return goodIn();
}

bool BeepSessionMgr::skipStrIfMatch(const char *str, int len) {
	if (goodIn()) {
		if (theInBuf.contSize() < len)
			errorNoContent();
		else
		if (strncmp(theInBuf.content(), str, len) == 0)
			theInBuf.consumed(len);
		else
			return false; // not an error
	}
	return goodIn();
}

bool BeepSessionMgr::skipSpace(int len) {
	Assert(len > 0);
	if (goodIn()) {
		if (theInBuf.contSize() < len)
			return errorNoContent();
		while (len-- && isspace(*theInBuf.content()))
			theInBuf.consumed(1);
		if (len > 0)
			error("malformed incoming data, expecting a space");
	}
	return goodIn();
}

bool BeepSessionMgr::skipCrLf() {
	return skipStrIfMatch("\r\n", 2) || skipStrIfMatch("\n", 1);
}

bool BeepSessionMgr::skipTrailer() {
	if (skipStrIfMatch(TheBeepFrameTrailer.cstr(), TheBeepFrameTrailer.len())) {
		if (skipCrLf())
			return true;
	}
	if (goodIn())
		error("malformed message trailer");
	return false;
}
	
	

int BeepSessionMgr::getInt() {
	int n = -1;
	if (goodIn()) {
		// MS VC++ requires istreams to work with modifiable buffers
		istringstream is(std::string(theInBuf.content(), theInBuf.contSize()));
		if (is >> n)
			theInBuf.consumed(Size(is.tellg()));
		else
			error("malformed incoming data, expecting an integer");
	}
	return goodIn() ? n : -1;
}

String BeepSessionMgr::getStr(int len) {
	if (goodIn()) {
		if (len <= theInBuf.contSize()) {
			String str;
			str.append(theInBuf.content(), len);
			theInBuf.consumed(len);
			return str;
		} else {
			errorNoContent();
		}
	}
	return String();
}

bool BeepSessionMgr::error(const String &reason) {
	//cerr << here << "error: " << reason << endl;
	theError = reason;
	return false;
}

bool BeepSessionMgr::errorNoSpace() {
	//cerr << here << "errorNoSpace" << endl;
	needMoreSpace = true;
	return false;
}

bool BeepSessionMgr::errorNoContent() {
	//cerr << here << "errorNoContent" << endl;
	needMoreContent = true;
	return false;
}

bool BeepSessionMgr::goodOut() const {
	return !needMoreSpace && !theError;
}

bool BeepSessionMgr::goodIn() const {
	return !needMoreContent && !theError;
}
