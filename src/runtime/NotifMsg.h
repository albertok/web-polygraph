
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_NOTIFMSG_H
#define POLYGRAPH__RUNTIME_NOTIFMSG_H

#include "xstd/NetDouble.h"

class NetAddr;

// messages used to send notifications to some [remote] monitor

class NotifMsg {
	public:
		enum Type { mtNone, mtStatus };

	public:
		NotifMsg(int anId = mtNone): theId(anId), theSize(sizeof(NotifMsg)) {};

	protected:
		void hton();
		void ntoh();

	public:
		int theId;
		int theSize;
};

class StatusNotifMsg: public NotifMsg {
	public:
		StatusNotifMsg(const String &aLabel = 0);

		void hton();
		void ntoh();

	public:
		char theLabel[16];     // a short 0-terminated description of an experiment
		Time theStartTime;
		Time theSndTime;       // when the message is sent
		Time theRespTime;
		NetDouble theReqRate;  // req / sec
		NetDouble theRepRate;  // rep / sec
		NetDouble theBwidth;   // network I/O bandwidth (r/w) [B/s]
		NetDouble theDHR;
		NetDouble theConnUse;  // req / connection
		NetDouble theErrRatio;

		Counter theXactTotCnt;  // total so far
		Counter theErrTotCnt;   // total so far
		Counter theSockInstCnt; // at the moment
		int theCat;             // entry category (aka "side")
};

// status message forwarded by the first recepient
class StatusFwdMsg: public StatusNotifMsg {
	public:
		StatusFwdMsg();
		StatusFwdMsg(const StatusNotifMsg &m, Time aRcvTime, const NetAddr &aSndAddr);

		void hton();
		void ntoh();

	public:
		Time theRcvTime;       // when the message is received
		struct {               // original sender (set by first recepient)
			struct sockaddr_storage addr;
			int port;
		} theSndAddr;
		int theCopyCnt;        // number of copies forwarded (to various recepients)
};

#endif
