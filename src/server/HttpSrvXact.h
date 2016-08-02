
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__SERVER_HTTPSRVXACT_H
#define POLYGRAPH__SERVER_HTTPSRVXACT_H

#include "base/ObjTimes.h"
#include "server/SrvXact.h"

class HttpSrvXact: public SrvXact {
	public:
		HttpSrvXact();

		virtual void reset();

		virtual int cookiesSent() const;
		virtual int cookiesRecv() const;

	protected:
		virtual void doStart();
		virtual void logStats(OLog &ol) const;
		virtual void noteBodyDataReady();
		virtual void noteBufReady();

		virtual void noteHdrDataReady();
		virtual void noteRepSent();
		virtual void makeRep(WrBuf &buf);

		Error interpretHeader();
		bool cfgAbortedReq() const;
		Error checkUri();
		Error setViserv(const NetAddr &name);
		Error setTarget(const NetAddr &target);
		void normalizeRanges();

		void make100Continue(HttpPrinter &hp);
		void make2xxContent(HttpPrinter &hp);
		bool canMake302Found(ObjId &oid) const;
		bool make302Found(HttpPrinter &hp);
		bool shouldMake302Found() const;
		void make304NotMod(HttpPrinter &hp);
		bool shouldMake304NotMod() const;
		void make406NotAcceptable(HttpPrinter &hp);
		void make416RequestedRangeNotSatisfiable(HttpPrinter &hp);
		bool shouldMake416RequestedRangeNotSatisfiable() const;
		void make417ExpectationFailed(HttpPrinter &hp);

		void putResponseLine(ostream &os, const String &suffix);
		void putStdFields(HttpPrinter &hp) const;
		void putXFields(HttpPrinter &hp) const;
		void put2xxContentHead(HttpPrinter &hp);
		void putRemWorld(HttpPrinter &hp, const ObjWorld &oldWorld) const;
		void putCookies(ostream &os);

		void openSimpleMessage(HttpPrinter &hp, const int status, const String &header, const String *const body);
		void closeSimpleMessage(ostream &os, const String *const body);

		virtual void doPhaseSync(const MsgHdr &hdr) const;

	protected:
		ReqHdr theReqHdr;

		HttpVersion theHttpVersion;
		ObjTimes theTimes;    // mod, lmt, and such

		Size theProducedSize;

		RangeList theRanges; // Only satisfiable a-b ranges.

		// the states below are implicitly mutually exclusive
		enum {
			csNone, // no request yet or no Expect: 100-continue request header
			csAllowed, // we plan to send 100 Continue control message
			csDenied, // we plan to send 417 Expectation Failed response
			csDone // we sent or do not plan on sending a 100 or 417 response
		} the100ContinueState;

		int theCookiesSentCount; // number of cookies sent in response

	private:
		Size theConsumedSize;
};

#endif

