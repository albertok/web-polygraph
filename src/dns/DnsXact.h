
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__DNS_DNSXACT_H
#define POLYGRAPH__DNS_DNSXACT_H

#include "xstd/Time.h"
#include "xstd/AlarmClock.h"

class CltXact; // DnsXact treats this as void
class DnsResp;
class DnsMgr;

// a single query-response DNS transaction
class DnsXact: public AlarmUser {
	public:
		DnsXact();
		virtual ~DnsXact();

		virtual void reset();

		void exec(DnsMgr *anOwner, const NetAddr &ns);
		void retry(const NetAddr &ns);

		void reason(CltXact *aReason) { theReason = aReason; }
		void queryAddr(const NetAddr &anAddr) { theQueryAddr = anAddr; }
		void timeout(Time aTout) { theTimeout = aTout; }
		void type(DnsMsg::Types type) { theType = type; }
		void idx(int anIdx) { theIdx = anIdx; }

		const NetAddr &queryAddr() const { return theQueryAddr; }
		const NetAddr &respAddr() const { return theRespAddr; }
		CltXact *reason() { return theReason; }
		int id() const { return theId; }
		int idx() const { return theIdx; }

		bool needRetry() const { return doRetry; }
		int tryCount() const { return theTryCount; }

		void noteReply(const DnsResp &rep);

		virtual void wakeUp(const Alarm &);

		int logCat() const;

	protected:
		void exec(const NetAddr &ns);
		void finish();

	protected:
		DnsMgr *theOwner;
		CltXact *theReason;

		NetAddr theQueryAddr;
		NetAddr theRespAddr;

		Time theTimeout;

		int theId;
		int theIdx;

		int theTryCount;
		bool doRetry;
		DnsMsg::Types theType;
};

#endif
