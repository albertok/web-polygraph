
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"
#include "dns/DnsMgr.h"
#include "dns/DnsQuery.h"
#include "dns/DnsResp.h"
#include "dns/DnsXact.h"


DnsXact::DnsXact() {
	reset();
}

DnsXact::~DnsXact() {
	Assert(!theOwner);
}

// note: keep in syn with partial reset in DnsXact::retry()!
void DnsXact::reset() {
	Assert(!thePendAlarmCnt);

	theOwner = 0;
	theReason = 0;
	theQueryAddr = theRespAddr = NetAddr();
	theTimeout = Time();
	theId = theIdx = -1;

	theTryCount = 0;
	doRetry = false;
}

void DnsXact::exec(DnsMgr *anOwner, const NetAddr &ns) {
	Assert(!theOwner && anOwner);
	theOwner = anOwner;
	exec(ns);
}

void DnsXact::retry(const NetAddr &ns) {
	Assert(theOwner && theReason);

	// partial reset
	theRespAddr = NetAddr();
	doRetry = false;

	exec(ns);
}

void DnsXact::exec(const NetAddr &ns) {
	Assert(ns);

	theId = DnsMsg::NextId();
	theTryCount++;

	DnsQuery q(theType);
	q.queryAddr(theQueryAddr);
	q.id(theId);

	if (Should(q.sendTo(ns, theOwner->socket()))) {
		if (theTimeout >= 0)
			sleepFor(theTimeout);
	} else {
		// try again ASAP
		doRetry = true;
		sleepFor(Time(0,0)); 
	}
}

void DnsXact::wakeUp(const Alarm &a) {
	AlarmUser::wakeUp(a);

	// note: doRetry may be set in DnsXact::exec to postpone finish
	if (!doRetry) {
		doRetry = true;
		if (ReportError(errDnsRepTimeout)) {
			Comment << theOwner->addr() << " failed to lookup "
				<< theQueryAddr << " after " << theTryCount << " attempts; "
				<< "last req id: " << theId << endc;
		}
	}

	finish();
}

void DnsXact::noteReply(const DnsResp &rep) {
	cancelAlarms();
	if (rep.error()) {
		if (ReportError(rep.error()))
			Comment << "DNS error while resolving " << theQueryAddr << endc;
		// XXX: when to retry?
	} else
	if (rep.queryAddr().sameButPort(theQueryAddr)) {
		Assert(rep.answers().count() > 0);
		Should(rep.answers().count() == 1);
		theRespAddr = *rep.answers()[0];
		theRespAddr.port(theQueryAddr.port());
		doRetry = false;
	} else {
		if (ReportError(errDnsRepMismatch))
			Comment << "asked for " << theQueryAddr << ", got response for " << rep.queryAddr() << endc;
		doRetry = true;
	}

	finish();
}

void DnsXact::finish() {
	DnsMgr *owner = theOwner;
	theOwner = 0;
	owner->noteXactDone(this);
}


int DnsXact::logCat() const {
	Assert(theOwner);
	return theOwner->logCat();
}
