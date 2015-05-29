
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "xstd/PrefixIdentifier.h"
#include "xstd/NetAddr.h"
#include "xstd/gadgets.h"
#include "base/AddrParsers.h"


bool ParseNetAddr(const char *buf, const char *eoh, const int defaultPort, NetAddr &addr) {
	int port;

	// IPv6 addresses must be delimited by square brackets to
	// resolve conflicts with port numbers (1:2::3:80)

	const char *column = StrBoundRChr(buf, ':', eoh);
	const char *endIPv6 = StrBoundRChr(buf, ']', eoh);

	if (column && (!endIPv6 || column > endIPv6)) {
		if (isInt(column + 1, port))
			eoh = column;
		else
			return false; // malformed port number
	} else {
		Should(defaultPort >= 0);
		port = defaultPort;
		// remove CRLF (if :port is present that is done automagically)
		while (buf < eoh && eoh[-1] == '\n') --eoh;
		while (buf < eoh && eoh[-1] == '\r') --eoh;
		if (buf == eoh)
			return false;
	}

	addr.port(port);
	if (endIPv6)
		return ParseIPvSix(buf, endIPv6, addr);
	else
	if (ParseIPvFour(buf, eoh, addr))
		return true;
	else
	if (ParseDname(buf, eoh, addr))	// almost always true
		return true;
	return false;
}

bool ParseDname(const char *buf, const char *eoh, NetAddr &addr) {
	if (buf < eoh) {
		addr.addr(String(buf, eoh-buf));
		return true;
	}

	return false;
}

// convert a.b.c.d to an int address
// this is a poor man's inet_aton()...
bool ParseIPvFour(const char *buf, const char *eoh, NetAddr &addr) {
	if (!isdigit(*buf)) // most common case first
		return false;

	int acc = 0;
	if (!isInt(buf, acc, &buf))
		return false;
	for (int i = 2; i >= 0 && buf < eoh; --i) {
		if (*buf != '.')
			return false;
		int octet;
		if (!isInt(buf+1, octet, &buf))
			return false;
		if (octet < 0 || octet > 255)
			return false;
		acc <<= 8;
		acc |= octet;
	}

	if (buf != eoh)
		return false; // trailing garbage

	in_addr name;
	name.s_addr = htonl(acc);
	addr.addr(name);
	return true;
}

// buf is not 0-terminated and system functions that parse IP
// addresses (such as inet_pton()) require 0-termination.  So we
// have to either write our own parser or copy the value to a
// 0-termainted buffer.  We choose the latter.
bool ParseIPvSix(const char *buf, const char *eoh, NetAddr &addr) {
	if (*buf != '[') // most common case first
		return false;
	buf++;

	char copy[64];	// an IPv6 address should fit in 40 bytes
	Size len = eoh - buf;
	if (len >= SizeOf(copy))
		return false;
	strncpy(copy, buf, len); // will not zero-terminate
	copy[len] = '\0';

	struct in6_addr a;
	if (inet_pton(AF_INET6, copy, &a) != 1)
		return false;

	addr.addr(a);
	return true;
}

// returns null on failure or end of host
const char *SkipHostInUri(const char *buf, const char *eorl, NetAddr &host) {
	if (const char *n = StrBoundChr(buf, ':', eorl)) {
		if (*++n == '/' && *++n == '/') {
			static PrefixIdentifier sidf;
			if (!sidf.count()) {
				sidf.add("ftp:", 21);
				sidf.add("http:", 80);
				sidf.add("https:", 443);
				sidf.optimize();
			}
			const int defaultPort(sidf.lookup(buf, n - buf - 1));
			buf = n+1;
			const char *end = StrBoundChr(buf, '/', eorl);
			if (!end)
				end = eorl;

			if (ParseNetAddr(buf, end, defaultPort, host))
				return end;
		}
	}
	return 0;
}
