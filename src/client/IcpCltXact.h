
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_ICPCLTXACT_H
#define POLYGRAPH__CLIENT_ICPCLTXACT_H

#include "xstd/AlarmClock.h"
#include "xstd/NetAddr.h"
#include "base/ObjId.h"
#include "icp/IcpOpCode.h"
#include "runtime/IcpXaction.h"
#include "icp/IcpClient.h"

class CltXact;
class Client;
class IcpMsg;

// XXX: make CltXact a parent of HttpCltXact and IcpCltXact?

// base class for ICP query-reply sequence
class IcpCltXact: public IcpXaction, public IcpCltUser, public AlarmUser {
	public:
		static Time TheTimeout; // read-only, except for configuration

	public:
		IcpCltXact() { reset(); }

		virtual void reset();

		virtual const ObjId &oid() const { return IcpXaction::oid(); }
		CltXact *reason(Client *) { return theReason; }
		const CltXact *reason() const { return theReason; }
		void reason(CltXact *x) { theReason = x; }

		void exec(Client *anOwner, const NetAddr &addr);

		virtual void wakeUp(const Alarm &alarm);
		virtual void noteReply(const IcpMsg &r);

	protected:
		IcpClient *icpClient();
		void finish(Error err);

	protected:
		Client *theOwner;

	private:
		CltXact *theReason; // must be treated as const
};

#endif
