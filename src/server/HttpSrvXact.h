
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

		void make100Continue(ostream &os);
		void make2xxContent(ostream &os);
		bool canMake302Found(ObjId &oid) const;
		bool make302Found(ostream &os);
		bool shouldMake302Found() const;
		void make304NotMod(ostream &os);
		bool shouldMake304NotMod() const;
		void make406NotAcceptable(ostream &os);
		void make416RequestedRangeNotSatisfiable(ostream &os);
		bool shouldMake416RequestedRangeNotSatisfiable() const;
		void make417ExpectationFailed(ostream &os);

		void putResponseLine(ostream &os, const String &suffix);
		void putStdFields(ostream &os) const;
		void putXFields(ostream &os) const;
		void put2xxContentHead(ostream &os);
		void putRemWorld(ostream &os, const ObjWorld &oldSlice) const;
		void putCookies(ostream &os);

		void openSimpleMessage(ostream &os, const int status, const String &header, const String *const body);
		void closeSimpleMessage(ostream &os, const String *const body);

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
			csDone, // we sent or do not plan on sending a 100 or 417 response
		} the100ContinueState;

		int theCookiesSentCount; // number of cookies sent in response

	private:
		Size theConsumedSize;
};

#endif

