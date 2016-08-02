
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Endian.h"
#include "xstd/NetAddr.h"
#include <sys/socket.h>
#include "runtime/NotifMsg.h"

#include "xstd/gadgets.h"
#include "base/polyLogCats.h"

/* network <-> host byte order conversions */

inline
void hton(Time &t) {
	t.tv_sec = htonl(t.tv_sec);
	t.tv_usec = htonl(t.tv_usec);
}

inline
void ntoh(Time &t) {
	t.tv_sec = ntohl(t.tv_sec);
	t.tv_usec = ntohl(t.tv_usec);
}

inline
void hton(NetDouble &d) {
	d.mnt = (int) htonl(d.mnt);
	d.exp = (int) htonl(d.exp);
}

inline
void ntoh(NetDouble &d) {
	d.mnt = (int) ntohl(d.mnt);
	d.exp = (int) ntohl(d.exp);
}

/* NotifMsg */

void NotifMsg::hton() {
	theId = (int) htonl(theId);
	theSize = (int) htonl(theSize);
}

void NotifMsg::ntoh() {
	theId = (int) ntohl(theId);
	theSize = (int) ntohl(theSize);
}


/* StatusNotifMsg */

StatusNotifMsg::StatusNotifMsg(const String &aLabel): NotifMsg(mtStatus) {
	theSize = sizeof(*this);

	memset(theLabel, 0, sizeof(theLabel));
	if (aLabel)
		strncpy(theLabel, aLabel.cstr(), sizeof(theLabel));
	
	theXactTotCnt = theErrTotCnt = theSockInstCnt = -1;
	theCat = lgcAll;
}

void StatusNotifMsg::hton() {
	NotifMsg::hton();

	::hton(theStartTime);
	::hton(theSndTime);
	::hton(theRespTime);

	::hton(theReqRate);
	::hton(theRepRate);
	::hton(theBwidth);
	::hton(theDHR);
	::hton(theConnUse);
	::hton(theErrRatio);

	theXactTotCnt = htobe64(theXactTotCnt);
	theErrTotCnt = htobe64(theErrTotCnt);
	theSockInstCnt = htobe64(theSockInstCnt);
	theCat = htonl(theCat);
}

void StatusNotifMsg::ntoh() {
	NotifMsg::ntoh();

	::ntoh(theStartTime);
	::ntoh(theSndTime);
	::ntoh(theRespTime);

	::ntoh(theReqRate);
	::ntoh(theRepRate);
	::ntoh(theBwidth);
	::ntoh(theDHR);
	::ntoh(theConnUse);
	::ntoh(theErrRatio);

	theXactTotCnt = be64toh(theXactTotCnt);
	theErrTotCnt = be64toh(theErrTotCnt);
	theSockInstCnt = be64toh(theSockInstCnt);
	theCat = ntohl(theCat);
}


/* StatusFwdMsg */

StatusFwdMsg::StatusFwdMsg(): theCopyCnt(-1) {
	memset(&theSndAddr, 0, sizeof(theSndAddr));
	theSndAddr.port = -1;
}

StatusFwdMsg::StatusFwdMsg(const StatusNotifMsg &m, Time aRcvTime, const NetAddr &aSndAddr):
	StatusNotifMsg(m), theRcvTime(aRcvTime), theCopyCnt(-1) {

	theSndAddr.addr = aSndAddr.sockAddr();
	theSndAddr.port = aSndAddr.port();
}

void StatusFwdMsg::hton() {
	StatusNotifMsg::hton();

	::hton(theRcvTime);

	// addr_in is always in network byte order?
	theSndAddr.port = (int) htonl(theSndAddr.port);

	theCopyCnt = (int) htonl(theCopyCnt);
}

void StatusFwdMsg::ntoh() {
	StatusNotifMsg::ntoh();

	::ntoh(theRcvTime);

	// addr_in is always in network byte order?
	theSndAddr.port = (int) ntohl(theSndAddr.port);

	theCopyCnt = (int) ntohl(theCopyCnt);
}
		
