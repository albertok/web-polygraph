
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/polyLogCats.h"
#include "base/StatPhaseRec.h"
#include "base/OLog.h"
#include "base/ILog.h"


StatPhaseRec::StatPhaseRec():
	theConnPipelineDepths(0,100),
	theCookiesPurgedFresh(0), theCookiesPurgedStale(0),
	theCookiesUpdated(0),
	primary(false) {
}

void StatPhaseRec::repAll(TmSzHistStat &all) const {
	TmSzHistStat imsAll;
	imsAll += theImsXacts.hits();
	imsAll += theImsXacts.misses();

	TmSzHistStat basicAll;
	basicAll += theBasicXacts.hits();
	basicAll += theBasicXacts.misses();

	TmSzHistStat ftpAll;
	ftpAll += theFtpXacts.hits();
	ftpAll += theFtpXacts.misses();

	all += basicAll;
	all += ftpAll;
	all += theRediredReqXacts;
	all += theRepToRedirXacts;
	all += imsAll;
	all += theReloadXacts;
	all += theHeadXacts;
	all += thePostXacts;
	all += thePutXacts;
	all += theConnectXacts;
	all += theRangeXacts;
}

void StatPhaseRec::compoundAll(CompoundXactStat &all) const {
	all += theAuthBasic;
	all += theAuthNtlm;
	all += theAuthNegotiate;
	all += theAuthKerberos;
	all += theConnected;
}

OLog &StatPhaseRec::store(OLog &log) const {
	StatIntvlRec::store(log);
	return log 
		<< theName 
		<< theGoal
		<< theRepContTypeHist
		<< theConnClose
		<< theSockRdSzH << theSockWrSzH
		<< theBasicXacts << theRediredReqXacts << theRepToRedirXacts 
		<< theImsXacts << theReloadXacts
		<< theHeadXacts << thePostXacts << thePutXacts << theConnectXacts
		<< theIcpXacts
		<< theOidGen
		<< theErrors
		<< theConnPipelineDepths
		<< thePageHist
		<< theRangeXacts
		<< theRangeGen
		<< theReqContTypeHist
		<< theProxyValidationR
		<< theLastReqByteWritten
		<< theFirstRespByteRead
		<< theLastReqByteRead
		<< theFirstRespByteWritten
		<< theAuthBasic
		<< theAuthNtlm
		<< theAuthNegotiate
		<< theAuthKerberos
		<< theConnected
		<< theSingles
		<< primary
		<< theCookiesSent << theCookiesRecv
		<< theCookiesPurgedFresh << theCookiesPurgedStale
		<< theCookiesUpdated
		<< theFtpXacts
		<< theStatusCode
		<< theCustomXacts
		<< theSslSessions
		;
}

ILog &StatPhaseRec::load(ILog &log) {
	StatIntvlRec::load(log);
	return log
		>> theName
		>> theGoal
		>> theRepContTypeHist
		>> theConnClose
		>> theSockRdSzH >> theSockWrSzH
		>> theBasicXacts >> theRediredReqXacts >> theRepToRedirXacts 
		>> theImsXacts >> theReloadXacts
		>> theHeadXacts >> thePostXacts >> thePutXacts >> theConnectXacts
		>> theIcpXacts
		>> theOidGen
		>> theErrors
		>> theConnPipelineDepths
		>> thePageHist
		>> theRangeXacts
		>> theRangeGen
		>> theReqContTypeHist
		>> theProxyValidationR
		>> theLastReqByteWritten
		>> theFirstRespByteRead
		>> theLastReqByteRead
		>> theFirstRespByteWritten
		>> theAuthBasic
		>> theAuthNtlm
		>> theAuthNegotiate
		>> theAuthKerberos
		>> theConnected
		>> theSingles
		>> primary
		>> theCookiesSent >> theCookiesRecv
		>> theCookiesPurgedFresh >> theCookiesPurgedStale
		>> theCookiesUpdated
		>> theFtpXacts
		>> theStatusCode
		>> theCustomXacts
		>> theSslSessions
		;
}

void StatPhaseRec::concat(const StatPhaseRec &r) {
	if (theName && r.theName && theName != r.theName)
		theName += "|" + r.theName;
	StatIntvlRec::concat(r);
	theGoal.concat(r.theGoal);
	join(r);
}

void StatPhaseRec::merge(const StatPhaseRec &r) {
	if (theName && r.theName && theName != r.theName)
		theName += "&" + r.theName;
	StatIntvlRec::merge(r);
	theGoal.merge(r.theGoal);
	join(r);
}

void StatPhaseRec::join(const StatPhaseRec &r) {
	if (!theName)
		theName = r.theName;
	if (!primary)
		primary = r.primary;

	theRepContTypeHist.add(r.theRepContTypeHist);
	theReqContTypeHist.add(r.theReqContTypeHist);
	theConnClose.add(r.theConnClose);
	theSockRdSzH.add(r.theSockRdSzH);
	theSockWrSzH.add(r.theSockWrSzH);
	theBasicXacts += r.theBasicXacts;
	theRediredReqXacts += r.theRediredReqXacts;
	theRepToRedirXacts += r.theRepToRedirXacts;
	theImsXacts += r.theImsXacts;
	theReloadXacts += r.theReloadXacts;
	theHeadXacts += r.theHeadXacts;
	thePostXacts += r.thePostXacts;
	thePutXacts += r.thePutXacts;
	theConnectXacts += r.theConnectXacts;
	theConnPipelineDepths += r.theConnPipelineDepths;
	thePageHist += r.thePageHist;
	theIcpXacts  += r.theIcpXacts;
	theOidGen += r.theOidGen;
	theErrors.add(r.theErrors);
	theRangeXacts += r.theRangeXacts;
	theRangeGen += r.theRangeGen;
	theProxyValidationR += r.theProxyValidationR;
	theLastReqByteWritten += r.theLastReqByteWritten;
	theFirstRespByteRead += r.theFirstRespByteRead;
	theLastReqByteRead += r.theLastReqByteRead;
	theFirstRespByteWritten += r.theFirstRespByteWritten;
	theAuthBasic += r.theAuthBasic;
	theAuthNtlm += r.theAuthNtlm;
	theAuthNegotiate += r.theAuthNegotiate;
	theAuthKerberos += r.theAuthKerberos;
	theConnected += r.theConnected;
	theSingles += r.theSingles;
	theCookiesSent += r.theCookiesSent;
	theCookiesRecv += r.theCookiesRecv;
	theCookiesPurgedFresh += r.theCookiesPurgedFresh;
	theCookiesPurgedStale += r.theCookiesPurgedStale;
	theCookiesUpdated += r.theCookiesUpdated;
	theFtpXacts += r.theFtpXacts;
	theStatusCode.add(r.theStatusCode);
	theCustomXacts += r.theCustomXacts;
	theSslSessions += r.theSslSessions;
}

ostream &StatPhaseRec::print(ostream &os, const String &pfx) const {
	os << "name:\t " << theName << endl;
	os << "primary:\t " << primary << endl;

	StatIntvlRec::print(os, pfx);

	theGoal.print(os, pfx + "goal.");

	TmSzHistStat imsAll;
	imsAll += theImsXacts.hits();
	imsAll += theImsXacts.misses();

	TmSzHistStat basicAll;
	basicAll += theBasicXacts.hits();
	basicAll += theBasicXacts.misses();

	TmSzHistStat ftpAll;
	ftpAll += theFtpXacts.hits();
	ftpAll += theFtpXacts.misses();

	TmSzHistStat all;
	this->repAll(all);

	all.print(os, pfx + "rep.");
	basicAll.print(os, pfx + "basic.");
	imsAll.print(os, pfx + "ims.");
	theBasicXacts.print(os, "hit", "miss", pfx);
	theRediredReqXacts.print(os, pfx + "redired_req.");
	theRepToRedirXacts.print(os, pfx + "rep_to_redir.");
	theImsXacts.print(os, "sc200", "sc304", pfx + "ims.");
	theReloadXacts.print(os, pfx + "reload.");
	theHeadXacts.print(os, pfx + "head.");
	thePostXacts.print(os, pfx + "post.");
	thePutXacts.print(os, pfx + "put.");
	theConnectXacts.print(os, pfx + "connect.");
	theConnPipelineDepths.print(os, pfx + "conn.pipeline.depth.");
	thePageHist.print(os, pfx + "page.");

	theIcpXacts.print(os, "hit", "miss",  pfx + "icp.");

	theRepContTypeHist.print(os, pfx + "cont_type.");
	theReqContTypeHist.print(os, pfx + "req_cont_type.");

	theConnClose.print(os, pfx + "conn_close.");
	theSockRdSzH.print(os, pfx + "so_read.size.");
	theSockWrSzH.print(os, pfx + "so_write.size.");

	theOidGen.print(os, pfx + "oid_gen.");
	theErrors.print(os, pfx + "errors.");

	theRangeXacts.print(os, pfx + "range.");
	theRangeGen.print(os, pfx + "range_gen.");

	TmSzHistStat proxyValidationAll;
	proxyValidationAll += theProxyValidationR.hits();
	proxyValidationAll += theProxyValidationR.misses();
	proxyValidationAll.print(os, pfx + "proxy_validation.");
	theProxyValidationR.print(os, "useful", "useless", pfx + "proxy_validation.");

	theLastReqByteWritten.print(os, pfx + "last_req_byte_written.");
	theFirstRespByteRead.print(os, pfx + "first_resp_byte_read.");
	theLastReqByteRead.print(os, pfx + "last_req_byte_read.");
	theFirstRespByteWritten.print(os, pfx + "first_resp_byte_written.");

	theAuthBasic.print(os, pfx + "compound.auth.basic.");
	theAuthNtlm.print(os, pfx + "compound.auth.ntlm.");
	theAuthNegotiate.print(os, pfx + "compound.auth.negotiate.");
	theAuthKerberos.print(os, pfx + "compound.auth.kerberos.");
	theConnected.print(os, pfx + "compound.connect_plus_one.");
	theSingles.print(os, pfx + "compound.not.");

	theCookiesSent.print(os, pfx + "cookies.sent.");
	theCookiesRecv.print(os, pfx + "cookies.recv.");
	os << pfx << "cookies.purged.fresh: \t " << theCookiesPurgedFresh << endl;
	os << pfx << "cookies.purged.stale: \t " << theCookiesPurgedStale << endl;
	os << pfx << "cookies.updated: \t " << theCookiesUpdated << endl;

	ftpAll.print(os, pfx + "ftp.");
	theFtpXacts.print(os, "active", "passive", pfx + "ftp.");

	theStatusCode.print(os, pfx + "rep_status_code.");

	theCustomXacts.print(os, pfx + "custom.");

	theSslSessions.print(os, pfx + "ssl.sessions.");

	return os;
}

void StatPhaseRec::recordByteTime(const int logCat, const Time &req, const Time &resp) {
	Assert(logCat == lgcCltSide || logCat == lgcSrvSide);
	if (req > 0) {
		TimeHist &lastReqByteStat = (logCat == lgcCltSide) ? theLastReqByteWritten : theLastReqByteRead;
		lastReqByteStat.record(req);
	}
	if (resp > 0) {
		TimeHist &firstRespByteStat = (logCat == lgcCltSide) ? theFirstRespByteRead : theFirstRespByteWritten;
		firstRespByteStat.record(resp);
	}
}
