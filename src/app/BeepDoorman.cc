
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "beep/RawBeepMsg.h"
#include "beep/BeepSessionMgr.h"
#include "xml/XmlSearch.h"
#include "runtime/LogComment.h"
#include "runtime/globals.h"
#include "app/BeepDoorman.h"

static const String ThePolyBeepProfileUri =
	"http://www.web-polygraph.org/beep/profiles/basic/";

static const String ThePolyBeepProfile =
	String("<profile uri='") + ThePolyBeepProfileUri + "' />";

static const String TheGreetMsgText =
	String("<greeting>") + ThePolyBeepProfile + "</greeting>";

static const String TheGreetRpyText =
	"</greeting>";


BeepDoorman::BeepDoorman() {
}

BeepDoorman::~BeepDoorman() {
	if (theListReserv)
		TheFileScanner->clearRes(theListReserv);
	if (theListSock)
		theListSock.close();
	while (theSessions.count())
		abortSession(theSessions.last());
}

void BeepDoorman::configure(const NetAddr &aListAddr, const NetAddr &aFwdAddr) {
	Assert(aListAddr || aFwdAddr);
	Assert(!theListAddr && !theFwdAddr);
	theListAddr = aListAddr;
	theFwdAddr = aFwdAddr;
}

void BeepDoorman::start() {
	if (theListAddr) {
		Must(theListSock.create(theListAddr.addrN().family()));
		Should(theListSock.reuseAddr(true));
		Must(theListSock.blocking(false));
		Must(theListSock.bind(theListAddr));
		Must(theListSock.listen());
		theListReserv = TheFileScanner->setFD(theListSock.fd(), dirRead, this);
	}

	if (theFwdAddr) {
		Socket fwd;
		Must(fwd.create(theFwdAddr.addrN().family()));
		Must(fwd.blocking(false));
		if (Should(fwd.connect(theFwdAddr))) {
			startSession(fwd, theFwdAddr);
		} else {
			Comment << "error: doorman failed to connect to"
				<< " the notification server at " << theFwdAddr << endc;
			fwd.close();
		}
	}
}

void BeepDoorman::startSession(Socket &sock, const NetAddr &them) {
	SessionRec s;
	s.theSock = sock;
	s.theRAddr = them;
	s.theMgr = new BeepSessionMgr(sock.fd());
	s.theRdRes = TheFileScanner->setFD(sock.fd(), dirRead, this);
	s.theIdx = theSessions.count();
	s.theMgr->startChannel(0, ThePolyBeepProfileUri);
	theSessions.append(s);
	putMsg(theSessions.last(), genGreetingMsg());
}

void BeepDoorman::noteReadReady(int fd) {
	if (theListSock.fd() == fd)
		accept();
	else
		readFrom(fd);
}

void BeepDoorman::noteWriteReady(int fd) {
	for (int idx = 0; idx < theSessions.count(); ++idx) {
		if (theSessions[idx].theSock.fd() == fd) {
			writeFor(theSessions[idx]);
			return;
		}
	}
	Check(false);
}

void BeepDoorman::accept() {
	NetAddr them;
	Socket cl = theListSock.accept(them);
	if (Should(cl)) {
		Should(cl.blocking(false));
		startSession(cl, them);
	}
}

void BeepDoorman::readFrom(int fd) {
	for (int idx = 0; idx < theSessions.count(); ++idx) {
		if (theSessions[idx].theSock.fd() == fd) {
			readFor(theSessions[idx]);
			return;
		}
	}
	Check(false);
}

void BeepDoorman::readFor(SessionRec &s) {
	Assert(s.theMgr);
	Assert(s.theMgr->hasSpaceIn());

	Size spaceSz;
	char *space = s.theMgr->spaceIn(spaceSz);
	const Size contSz = s.theSock.read(space, spaceSz);
	
	if (contSz < 0) {
		if (Error::LastExcept(EWOULDBLOCK)) {
			Should(false);
			abortSession(s);
		}
		return;
	}

	if (contSz == 0) {
		abortSession(s);
		return;
	}

	s.theMgr->spaceInUsed(contSz);

	if (!s.theMgr->hasSpaceIn() && s.theRdRes)
		TheFileScanner->clearRes(s.theRdRes);
	if (s.theMgr->hasContentOut() && !s.theWrRes)
		s.theWrRes = TheFileScanner->setFD(s.theSock.fd(), dirWrite, this);
	Assert(s.theRdRes || s.theWrRes);

	processMsgs(s);
}

void BeepDoorman::writeFor(SessionRec &s) {
	Assert(s.theMgr);
	Assert(s.theMgr->hasContentOut());

	Size contSz;
	const char *cont = s.theMgr->contentOut(contSz);
	const Size spaceSz = s.theSock.write(cont, contSz);

	if (spaceSz < 0) {
		if (Error::LastExcept(EWOULDBLOCK)) {
			Should(false);
			abortSession(s);
		}
		return;
	}

	if (spaceSz == 0)
		return;

	s.theMgr->contentOutUsed(spaceSz);

	if (!s.theMgr->hasContentOut() && s.theWrRes)
		TheFileScanner->clearRes(s.theWrRes);
	if (s.theMgr->hasSpaceIn() && !s.theRdRes)
		s.theRdRes = TheFileScanner->setFD(s.theSock.fd(), dirRead, this);

	Assert(s.theRdRes || s.theWrRes);
}

void BeepDoorman::processMsgs(SessionRec &s) {
	RawBeepMsg msg;
	while (s.theMgr->getMsg(msg)) {
		processMsg(s, msg);
	}
}

void BeepDoorman::processMsg(SessionRec &s, RawBeepMsg &msg) {
	//cerr << here << "recv: (" << msg.type() << ") " << msg.image() << endl;
	if (msg.channel() == 0) { // control channel
		if (msg.type() == RawBeepMsg::bmtRpy) {
			if (msg.no() == 0 && msg.image().str("<greeting")) {
				return; // no reply is needed?
			}
		} else
		if (msg.type() == RawBeepMsg::bmtMsg) {
			XmlSearch xs(msg.image());
			int chId = -1;
			if (xs.getInt("/start@number", chId)) {
				String profUri;
				if (xs.getStr("/start/profile@uri", profUri)) {
					if (profUri == ThePolyBeepProfileUri) {
						RawBeepMsg rpy(RawBeepMsg::bmtRpy);
						rpy.replyTo(msg);
						rpy.image(ThePolyBeepProfile);
						putMsg(s, rpy);
						const bool doPushFwd = theFwdImage && 
							s.theRAddr == theFwdAddr &&
							s.theMgr->channelCount() == 1;
						s.theMgr->startChannel(chId, profUri);
						if (doPushFwd)
							pushFwd(s);
						return;
					}
				}
			}
		}
	}
	cerr << here << "error: foreign BEEP message: (" << msg.type() << "): " << msg.image() << endl;
	Should(false); // XXX: implement
}

void BeepDoorman::bcastMsg(const String &image) {
	for (int i = 0; i < theSessions.count(); ++i) {
		SessionRec &s = theSessions[i];

		// queue for forwarding session if no data channels yet
		if (s.theRAddr == theFwdAddr && s.theMgr->channelCount() == 1) {
			if (Should(theFwdImage.len() < Size::MB(1))) // use Check()
				theFwdImage += image; // XXX: not a real queue
			else
				theFwdImage = image;  // cut if the queue grew too long
		}

		// skip control channel
		for (int c = 1; c < s.theMgr->channelCount(); ++c) {
			if (theFwdImage && s.theRAddr == theFwdAddr && c == 1) {
				theFwdImage += image;
				pushFwd(s);
			} else {
				RawBeepMsg msg(RawBeepMsg::bmtAns);
				msg.channel(s.theMgr->channelIdAt(c));
				msg.image(image);
				putMsg(s, msg);
			}
		}
	}
}

void BeepDoorman::pushFwd(SessionRec &s) {
	Assert(s.theMgr->channelCount() > 1);
	RawBeepMsg msg(RawBeepMsg::bmtAns);
	msg.channel(s.theMgr->channelIdAt(1));
	if (Should(s.theRAddr == theFwdAddr && theFwdImage)) {
		// forward delayed messages as one message
		msg.image(theFwdImage);
		theFwdImage = 0;
	}
	putMsg(s, msg);
}

void BeepDoorman::putMsg(SessionRec &s, const RawBeepMsg &msg) {
	//cerr << here << "sent: (" << msg.type() << ") " << msg.image() << endl;
	if (!s.theMgr->putMsg(msg))
		cerr << here << "error: failed to buffer a BEEP message" << endl;
	if (s.theMgr->hasContentOut() && !s.theWrRes)
		s.theWrRes = TheFileScanner->setFD(s.theSock.fd(), dirWrite, this);
}

RawBeepMsg BeepDoorman::genGreetingMsg() const {
	RawBeepMsg m(RawBeepMsg::bmtRpy);
	m.channel(0);
	m.image(TheGreetMsgText);
	return m;
}

void BeepDoorman::abortSession(SessionRec &s) {
	Assert(0 <= s.theIdx && s.theIdx < theSessions.count());
	s.theSock.close();
	delete s.theMgr;
	if (s.theRdRes)
		TheFileScanner->clearRes(s.theRdRes);
	if (s.theWrRes)
		TheFileScanner->clearRes(s.theWrRes);

	if (s.theIdx != theSessions.count() - 1) {
		const int idx = s.theIdx;
		theSessions[idx] = theSessions.pop();
		theSessions[idx].theIdx = idx;
	} else {
		theSessions.pop();
	}
}

