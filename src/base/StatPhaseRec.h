
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_STATPHASEREC_H
#define POLYGRAPH__BASE_STATPHASEREC_H

#include "base/StatIntvlRec.h"
#include "base/GoalRec.h"
#include "base/CompoundXactStat.h"
#include "base/ConnCloseStat.h"
#include "base/HRHistStat.h"
#include "base/ErrorStat.h"
#include "base/OidGenStat.h"
#include "base/histograms.h"
#include "base/RangeGenStat.h"
#include "base/SslPhaseStat.h"
#include "base/StatusCodeStat.h"

class StatPhaseMgr;

// accumulates all phase stats that are not handled by StatIntvlRec

class StatPhaseRec: public StatIntvlRec {
	public:
		StatPhaseRec();
	
		const String &name() const { return theName; }

		void repAll(TmSzHistStat &all) const;
		void compoundAll(CompoundXactStat &all) const;

		virtual OLog &store(OLog &log) const;
		virtual ILog &load(ILog &);

		void concat(const StatPhaseRec &p); // sequential phases
		void merge(const StatPhaseRec &p);  // concurrent phases

		ostream &print(ostream &os, const String &pfx) const;
		void recordByteTime(const int logCat, const Time &req, const Time &resp);

	protected:
		void join(const StatPhaseRec &p);

	public: // read-only
		String theName;
		GoalRec theGoal;  // maintained for us

		ContType::HistStat theReqContTypeHist; // request per-content-type stats
		ContType::HistStat theRepContTypeHist; // reply per-content-type stats
		ConnCloseStat theConnClose; // pre-conn-close-class stats

		SizeHist theSockRdSzH;   // socket reads
		SizeHist theSockWrSzH;   // socket writes

		/* HTTP */
		HRHistStat theBasicXacts;
		TmSzHistStat theRediredReqXacts;
		TmSzHistStat theRepToRedirXacts;
		HRHistStat theImsXacts;
		TmSzHistStat theReloadXacts;
		TmSzHistStat theHeadXacts;
		TmSzHistStat thePutXacts;
		TmSzHistStat thePostXacts;
		TmSzHistStat theConnectXacts;
		LineHist theConnPipelineDepths;
		TmSzHistStat thePageHist;
		HRHistStat theFtpXacts;
		SslPhaseStat theSslSessions; // SSL session stats

		/* ICP */
		HRHistStat theIcpXacts;

		OidGenStat theOidGen;
		ErrorStat theErrors;
		TmSzHistStat theRangeXacts;
		RangeGenStat theRangeGen;

		HRHistStat theProxyValidationR;

		TimeHist theLastReqByteWritten; // client side, last request byte written
		TimeHist theFirstRespByteRead; // client side, first response byte read
		TimeHist theLastReqByteRead; // server size, last request byte read
		TimeHist theFirstRespByteWritten; // server side, first response byte written

		/* compound transactions */
		CompoundXactStat theAuthBasic; // client side, basic auth
		CompoundXactStat theAuthNtlm; // client side, NTLM auth
		CompoundXactStat theAuthNegotiate; // client side, negotiate auth
		CompoundXactStat theAuthKerberos; // client side, kerberos auth
		CompoundXactStat theConnected; // client side, HTTP CONNECT + next xact
		CompoundXactStat theSingles; // transactions not a part of some compound

		/* Cookies */
		AggrStat theCookiesSent;   // cookies/msg for msgs sent w/ cookies
		AggrStat theCookiesRecv;   // cookies/msg for msgs received w/ cookies
		Counter theCookiesPurgedFresh; // number of fresh cookies purged
		Counter theCookiesPurgedStale; // number of stale cookies purged
		Counter theCookiesUpdated; // number of cookies updated

		StatusCodeStat theStatusCode; // response status code stats

		TmSzHistStat theCustomXacts; // custom stats replies

		bool primary; // true if the phase is primary
};

#endif
