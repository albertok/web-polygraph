
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/sstream.h"
#include "xstd/Socket.h"

#include "base/BStream.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "dns/DnsQuery.h"

static String DnsQueryStrName = "dnsquerystr";


bool DnsQuery::sendTo(const NetAddr &ns, Socket &s) {
	char buf[512];
	ofixedstream ostr(buf, sizeof(buf));
	OBStream os;
	os.configure(&ostr, DnsQueryStrName);
	
	Assert(dumpAQuery(os));
	Assert(os.offset() <= SizeOf(buf));

	return s.sendTo(buf, os.offset(), ns) == os.offset();
}

bool DnsQuery::dumpAQuery(OBStream &os) const {

	/* header section */

	os << id(); // XXX: check that we dump as short

	// XXX: check that we dump as short
	const Field flags = flagQuery | flagAQuery | flagRecursionDesired;
	os << flags;

	os << (Field)1; // QDCOUNT
	os << (Field)0; // ANCOUNT
	os << (Field)0; // NSCOUNT
	os << (Field)0; // ARCOUNT

	/* question section */

	dumpName(os, theQueryAddr);
	os << (Field)theType;
	os << (Field)classIn;

	return os.good();
}

bool DnsQuery::dumpName(OBStream &os, const NetAddr &addr) const {
	// <label-length> <label>
	for (StrIter i(addr.addrA(), '.'); i; ++i) {
		const int len = i.tokLen();
		Assert(0 < len && len < 64);
		os << (char)len;
		os.put(i.tokBeg(), len);
	}
	os << (char)0;
	return os.good();
}
