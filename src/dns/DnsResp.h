
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__DNS_DNSRESP_H
#define POLYGRAPH__DNS_DNSRESP_H

#include "xstd/Array.h"
#include "dns/DnsMsg.h"

class Socket;
class IBStream;

// common type for all DNS messages
class DnsResp: public DnsMsg {
	public:
		typedef Array<NetAddr*> Answers;

	public:
		DnsResp();

		void reset();

		bool recv(Socket &s);

		const Answers &answers() const { return theAnswers; }
		const Error &error() const { return theError; }

	protected:
		bool parseResp();
		bool parseName(IBStream &is, String &name);
		bool parseName(Size &off, String &name);

		void interpretRCode(Field rCode);

	protected:
		Answers theAnswers;
		Error theError;

	private: /* temporary info for parsing */
		const char *theBuf;
		int theBufSize;
};

#endif
