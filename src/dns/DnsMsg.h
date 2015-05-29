
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__DNS_DNSMSG_H
#define POLYGRAPH__DNS_DNSMSG_H

#include "xstd/NetAddr.h"

// common type for all DNS messages
class DnsMsg {
	public:
		enum RCodes { rcodeMask = 0x07, rcodeFmt = 1, rcodeSrvFail, rcodeNoName, rcodeNoSupp, rcodeRefused };
		enum Flags { flagQuery  = (0 << 15), flagResp = (1 << 15), flagAQuery = (0 << 11), flagRecursionDesired = (1 << 8) };
		enum Types { typeA = 1, typeAAAA = 28 };
		enum Classes { classIn = 1 };

		enum { LabelLenMax = 63 };

		typedef unsigned short Field;

	public:
		static Field LastId() { return TheLastId; }
		static Field NextId();

	private:
		static Field TheLastId;

	public:
		DnsMsg();

		void reset();

		void id(int anId) { theId = anId; }
		void queryAddr(const NetAddr &addr) { theQueryAddr = addr; }

		const NetAddr &queryAddr() const { return theQueryAddr; }
		Field id() const { return theId; }

	protected:
		NetAddr theQueryAddr;
		Field theId;
};

#endif
