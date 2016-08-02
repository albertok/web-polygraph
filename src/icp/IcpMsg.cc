
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/string.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/Socket.h"
#include "runtime/AddrMap.h"
#include "runtime/HostMap.h"
#include "runtime/ErrorMgr.h"
#include "runtime/httpHdrs.h"
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"
#include "runtime/globals.h"
#include "csm/oid2Url.h"
#include "icp/IcpMsg.h"


struct IcpRawMsg {
	typedef unsigned int u_num32; // XXX: check C++ standard and/or move this to config.h!

	unsigned int opCode:8;
	unsigned int version:8;
	unsigned int length:16;
	u_num32	reqNum;
	u_num32 _options;
	u_num32 _optData;
	u_num32 _senderHost;
	char buf[16*1024-5*32];
};


IcpMsg::IcpMsg(): theReqNum(-1), theOpCode(icpInvalid) {
}

bool IcpMsg::send(Socket &s) const {
	Assert(theAddr);

	static IcpRawMsg msg;
	const Size hdrSize = SizeOf(msg) - SizeOf(msg.buf);
	const Size urlOff = theOpCode == icpQuery ?
		SizeOf(msg._senderHost) : (Size)0;
	const Size bufLen = SizeOf(msg.buf) - urlOff - (Size)1;
	char *url = msg.buf + urlOff;

	memset(&msg, 0, SizeOf(msg));

	ofixedstream os(url, bufLen);
	Oid2Url(theOid, os);
	os << ends;
	const Size urlLen = strlen(url);

	msg.opCode = theOpCode;
	msg.version = 2;
	msg.length = hdrSize + urlOff + urlLen + (Size)1;
	msg.reqNum = (IcpRawMsg::u_num32) theReqNum;

	// preserve length
	const Size msgLen = (Size)msg.length;

	// ntoh
	msg.length = htons(msg.length);
	msg.reqNum = htonl(msg.reqNum);
	// we do not care about other fields for now

	return s.sendTo(&msg, msgLen, theAddr) == msgLen;
}

// just handy for quiting quickly
bool IcpMsg::finish(Error err) {
	if (err && err != errOther)
		ReportError(err);
	return false;
}

bool IcpMsg::receive(Socket &s) {
	static IcpRawMsg msg;
	const Size hdrSize = SizeOf(msg) - SizeOf(msg.buf);
	msg.buf[sizeof(msg.buf) - 1] = '\0';
	theSize = s.recvFrom(&msg, sizeof(msg) - 1, theAddr);

	if (theSize < 0)
		return finish(Error::LastExcept(EWOULDBLOCK));
	if (!theSize)
		return finish(Error());
	if (theSize < 2)
		return finish(errIcpMsgSize);

	// ntoh
	msg.length = ntohs(msg.length);
	msg.reqNum = ntohl(msg.reqNum);
	// we do not care about other fields for now

	if (msg.version != 2) // XXX: need to add v.3 for NetCAche
		return finish(errIcpVersion);

	if (theSize != (Size)msg.length || hdrSize > (Size)msg.length)
		return finish(errIcpMsgSize);

	Size urlOff = 0;
	bool isReply = true;

	switch (msg.opCode) {
		case icpHit:
		case icpMiss:
		case icpMissNoFetch:
			break;

		case icpQuery:
			urlOff += SizeOf(msg._senderHost); // skip client IP
			isReply = false;
			break;

		case icpError:
			return finish(errIcpErrOpcode);

		default: {
			if (ReportError(errIcpRepCode))
				Comment << "offending opcode: " << msg.opCode << endc;
			return finish(errOther);
		}
	}

	/* pasrse payload (URL) */
	msg.buf[Size(msg.length) - hdrSize] = '\0';
	const char *url = msg.buf + urlOff;

	//XXX: we need a --dump icp option
	//clog << here << "XXX: ICP: got URL: " << url << endl;

	ReqHdr h;
	if (!h.parseUri(url, url + msg.length - hdrSize - urlOff, h.theUri))
		return false;

	if (h.theUri.oid.foreignUrl())
		return finish(isReply ? errIcpForeignRep : errIcpForeignReq);

	if (!h.theUri.host) // probably failed to parse host part
		return finish(errNoHostName);

	// XXX: the lookups below are very slow
	if (!TheAddrMap->has(h.theUri.host))
		return finish(errForeignHostName);

	int viserv = -1;
	if (!TheHostMap->find(h.theUri.host, viserv))
		return finish(errForeignHostName);
	h.theUri.oid.viserv(viserv);
	h.theUri.oid.target(-1); // unknown

	// copy info
	theOid = h.theUri.oid;
	theReqNum = msg.reqNum;
	theOpCode = (IcpOpCode) msg.opCode;

	return true;
}
