
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "runtime/LogComment.h"
#include "base/StatIntvlRec.h"
#include "runtime/NotifMsg.h"
#include "runtime/Notifier.h"

#include "xstd/gadgets.h"


Notifier::Notifier(const String &aLabel, const NetAddr &anAddr):
	theLabel(aLabel), theAddr(anAddr) {

	bool ok = true;
	ok = ok && theSock.create(AF_INET, SOCK_DGRAM, 0);
	ok = ok && theSock.connect(theAddr);
	ok = ok && theSock.blocking(false);

	if (!ok) {
		Comment(2) << "warning: cannot connect to " << theAddr << " to send monitoring messages; possible cause: " << Error::Last() << endc;
		theSock.close();
	}
}

Notifier::~Notifier() {
	if (active())
		theSock.close();
}

bool Notifier::active() const {
	return theSock != 0;
}


void Notifier::noteStats(const StatIntvlRec &s, int cat) {
	if (!theSock)
		return;
	
	StatusNotifMsg msg(theLabel);

	msg.theStartTime = TheClock.TheStartTime;
	msg.theSndTime = TheClock;
	msg.theRespTime = Time::Msec((int)s.repTime().mean());
	msg.theReqRate = s.reqRate();
	msg.theRepRate = s.repRate();
	msg.theBwidth = Ratio(s.repSize().sum(), s.theDuration.secd());
	msg.theDHR = s.theRealHR.dhr();
	msg.theConnUse = s.theConnUseCnt.mean();
	msg.theErrRatio = s.errRatio();

	msg.theXactTotCnt = TheProgress.xacts();
	msg.theErrTotCnt = TheProgress.errs();
	msg.theSockInstCnt = Socket::Level();
	msg.theCat = cat;

	// write, ignoring errors
	msg.hton();
	theSock.write(&msg, sizeof(msg));
}
