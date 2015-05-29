
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__ICP_ICPCLIENT_H
#define POLYGRAPH__ICP_ICPCLIENT_H

#include "xstd/Array.h"
#include "icp/IcpBase.h"

class IcpCltRes {
	public:
		IcpCltRes(int anIdx = -1): theIdx(anIdx) {}

		void reset() { theIdx = -1; }

		operator void*() const { return theIdx >= 0 ? (void*)-1 : 0; }
		int reqNum() const { return theIdx; }

	protected:
		int theIdx;
};

class IcpCltUser {
	public:
		virtual ~IcpCltUser();
		virtual void reset();

		virtual const ObjId &oid() const = 0;
		virtual void noteReply(const IcpMsg &r) = 0;

	protected:
		IcpCltRes theReserv;
};

// matches ICP requests and responses
class IcpClient: virtual public IcpBase {
	public:
		IcpClient();

		IcpCltRes expectReply(IcpCltUser *u);
		void cancel(IcpCltRes &res);

	protected:
		virtual void noteReply(const IcpMsg &m);

		virtual int logCat() const;

	protected:
		Array<IcpCltUser*> theUsers;
		int theLastReqNum;
};

#endif
