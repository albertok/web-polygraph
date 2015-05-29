
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/sstream.h"
#include "xstd/Socket.h"

#include "base/BStream.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "dns/DnsResp.h"


static String DnsRespStrName = "dnsstr";


DnsResp::DnsResp(): theBuf(0), theBufSize(0) {
}

void DnsResp::reset() {
	DnsMsg::reset();

	while (theAnswers.count()) delete theAnswers.pop();
	theError = Error();

	theBuf = 0;
	theBufSize = 0;
}

bool DnsResp::recv(Socket &s) {
	char buf[512];
	theBufSize = s.read(buf, sizeof(buf));
	theBuf = buf;

	if (theBufSize <= 0)
		return false;

	if (!parseResp() && !theError)
		theError = errDnsRespFmt;
	return true;
}

bool DnsResp::parseResp() {

	/* header section */
	istringstream istr(string((char*)theBuf, theBufSize)); // VC++ requires a cast
	IBStream is;
	is.configure(&istr, DnsRespStrName);

	is >> theId;

	const Field flags = is.getUsi();

	if ((flags & flagResp) == 0)
		return false;

	if (const Field rCode = flags & rcodeMask)
		interpretRCode(rCode);

	Field qdCount, anCount, nsCount, arCount;
	is >> qdCount >> anCount >> nsCount >> arCount;
	if (qdCount != 1)
		return false;

	/* question section */
	String qName = "";
	if (!parseName(is, qName))
		return false;
	theQueryAddr = NetAddr(qName, -1);

	const Field qType = is.getUsi();
	const Field qClass = is.getUsi();
	if (qClass != classIn)
		return false;
	if (qType != typeA && qType != typeAAAA)
		return false;

	/* answer section */
	Assert(!theAnswers.count());
	theAnswers.stretch(anCount);
	while (anCount-- > 0) {
		String rName = "";
		if (!parseName(is, rName))
			return false;
		if (rName != qName)
			return false;

		const Field rType = is.getUsi();
		const Field rClass = is.getUsi();

		is.skip(4); // ttl;

		Field rdLen = is.getUsi();
		if (rType == typeA && rClass == classIn) {
			struct in_addr a;
			if (rdLen != sizeof(a))
				return false;
			is.get(&a, sizeof(a));
			theAnswers.append(new NetAddr(a, -1));
		} else
		if (rType == typeAAAA && rClass == classIn) {
			struct in6_addr a;
			if (rdLen != sizeof(a))
				return false;
			is.get(&a, sizeof(a));
			theAnswers.append(new NetAddr(a, -1));
		}
	}

	/* skip authority and additional info sections */
	return theAnswers.count();
}

bool DnsResp::parseName(IBStream &is, String &name) {
	Size off = is.offset();

	const bool res = parseName(off, name);

	is.skip(off - is.offset());
	return res;
}

bool DnsResp::parseName(Size &off, String &name) {
	while (off < theBufSize && theBuf[off]) {

		const Size len = (Size)(unsigned char)theBuf[off];

		if (len <= LabelLenMax) { // uncompressed label
			++off;
			if (theBufSize < off + len) // end of buffer
				return false;
			if (name)
				name += '.';
			name.append(theBuf+off, len);
			off += len;
		} else {                  // pointer to a label
			Field ptr;
			memcpy(&ptr, theBuf + off, sizeof(ptr));
			ptr = ntohs(ptr);
			ptr &= 0x3FFF;

			off += SizeOf(ptr);
			if (theBufSize <= ptr) // invalid pointer?
				return false;

			Size newOff = (Size)ptr;
			parseName(newOff, name);

			return true;
		}
	}

	if (off < theBufSize) {
		++off;
		return true;
	}

	return false;
}

void DnsResp::interpretRCode(Field rCode) {
	Assert(!theError);
	switch (rCode) {
		case 0:
			// no error
			break;
		case rcodeFmt:
			theError = errDnsQueryFmt;
			break;
		case rcodeSrvFail:
			theError = errDnsSrvFail;
			break;
		case rcodeNoName:
			theError = errDnsNoName;
			break;
		case rcodeNoSupp:
			theError = errDnsNoSupp;
			break;
		case rcodeRefused:
			theError = errDnsRefused;
			break;
		default:
			theError = errDnsRCode;
			break;
	}
}
