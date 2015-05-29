
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__ICP_ICPMSG_H
#define POLYGRAPH__ICP_ICPMSG_H

#include "xstd/NetAddr.h"
#include "xstd/Socket.h"
#include "base/ObjId.h"
#include "icp/IcpOpCode.h"

// common part of ICP requests and replies
class IcpMsg {
	public:
		IcpMsg();

		int logCat() const { return theLogCat; }
		IcpOpCode opCode() const { return theOpCode; }
		int reqNum() const { return theReqNum; }
		const ObjId &oid() const { return theOid; }
		const NetAddr &peer() const { return theAddr; }
		Size size() const { return theSize; }

		void logCat(int aCat) { theLogCat = aCat; }
		void opCode(IcpOpCode anOc) { theOpCode = anOc; }
		void oid(const ObjId &anOid) { theOid = anOid; }
		void reqNum(int aReqNum) { theReqNum = aReqNum; }
		void peer(const NetAddr &anAddr) { theAddr = anAddr; }

		bool send(Socket &s) const;
		bool receive(Socket &s);

	protected:
		bool finish(Error err);
		
	protected:
		NetAddr theAddr; // peer address
		ObjId theOid;
		int theReqNum;
		Size theSize;
		int theLogCat;
		IcpOpCode theOpCode;
};


#endif
