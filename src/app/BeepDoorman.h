
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__APP_BEEPDOORMAN_H
#define POLYGRAPH__APP_BEEPDOORMAN_H

#include "xstd/String.h"
#include "xstd/NetAddr.h"
#include "xstd/Array.h"
#include "app/BeepSessionRec.h"

class RawBeepMsg;

// accepts and dispatches incoming "control/meta level" BEEP connections
class BeepDoorman: public FileScanUser {
	public:
		typedef BeepSessionRec SessionRec;

	public:
		BeepDoorman();
		~BeepDoorman();

		void configure(const NetAddr &aListAddr, const NetAddr &aFwdAddr);

		void start(); // starts listening for connections

		void bcastMsg(const String &image);

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);

	protected:
		void startSession(Socket &sock, const NetAddr &them);

		void accept();
		void readFrom(int fd);
		void readFor(SessionRec &s);
		void writeFor(SessionRec &s);
		void processMsgs(SessionRec &s);
		void processMsg(SessionRec &s, RawBeepMsg &msg);
		void abortSession(SessionRec &s);
		void pushFwd(SessionRec &s);
		void putMsg(SessionRec &s, const RawBeepMsg &msg);

		RawBeepMsg genGreetingMsg() const;

	protected:
		NetAddr theListAddr;                // where to listen for conns
		NetAddr theFwdAddr;                 // where to forward messages
		Socket theListSock;
		FileScanReserv theListReserv;
		Array<BeepSessionRec> theSessions;  // open beep sessions
		String theFwdImage;                 // delayed images to theFwdAddr
};

#endif
