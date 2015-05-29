
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BEEP_BEEPSESSIONMGR_H
#define POLYGRAPH__BEEP_BEEPSESSIONMGR_H

#include "xstd/h/sstream.h"
#include "xstd/Array.h"
#include "xstd/String.h"
#include "runtime/IOBuf.h"

class RawBeepMsg;
class BeepChannel;

// BEEP (aka BXXP, RFC 3080) protocol wrapper
// manages a single BEEP session in an I/O model independent fashion
// does not implement most of the protocol
class BeepSessionMgr {
	public:
		typedef RawBeepMsg Msg;
		typedef BeepChannel Channel;

	public:
		BeepSessionMgr(int anId = -1);
		~BeepSessionMgr();

		int id() const { return theId; }
		int channelCount() const { return theChannels.count(); }
		int channelIdAt(int idx) const;

		void startChannel(int anId, const String &profile);

		// put/get next message
		bool putMsg(const Msg &msg);
		bool getMsg(Msg &msg);

		// I/O interfaces
		bool hasSpaceIn() const;
		bool hasSpaceOut() const;
		char *spaceIn(Size &size); // buffer to read-into (and its size)
		void spaceInUsed(Size size);  // how much was actually read
		bool hasContentIn() const;
		bool hasContentOut() const;
		const char *contentOut(Size &size) const; // buffer to write-from
		void contentOutUsed(Size size);  // how much was actually written

		bool goodOut() const;
		bool goodIn() const;
		bool good() const { return goodOut() && goodIn(); }

	protected:
		BeepChannel *findChannel(int chId);

		bool putStr(const String &s);
		bool putSpace();
		bool putChar(char ch);
		bool putInt(int n);
		bool skipStrIfMatch(const char *str, int len);
		bool skipSpace(int len = 1);
		bool skipChar(char ch);
		bool skipCrLf();
		bool skipTrailer();
		int getInt();
		String getStr(int len);

		bool errorNoContent();
		bool errorNoSpace();
		bool error(const String &reason);

	protected:
		Array<BeepChannel*> theChannels;
		WrBuf theInBuf;
		WrBuf theOutBuf;

		int theId;

		String theError;
		bool needMoreContent;
		bool needMoreSpace;
};

#endif
