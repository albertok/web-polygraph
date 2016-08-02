
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"

#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "probe/ProbeStatMgr.h"

ProbeStatMgr TheProbeStatMgr;
static const int lgProbeStats = lgEndOfSysTags+1; // assume no other objects are used


/* ProbeLinkRec */

ProbeLinkRec::ProbeLinkRec() {
}

ProbeLinkRec::ProbeLinkRec(const NetAddr &aCltHost, const NetAddr &aSrvHost):
	theCltHost(aCltHost), theSrvHost(aSrvHost) {
}

void ProbeLinkRec::store(OLog &log) const {
	log << theCltHost << theSrvHost;
	theStats.store(log);
}

void ProbeLinkRec::load(ILog &log) {
	log >> theCltHost >> theSrvHost;
	theStats.load(log);
}

void ProbeLinkRec::oneLineReport(ostream &os) const {
	//stats().theSockRdSzH.print(os << here, "sock_rd.size.") << endl;
	//stats().theSockWrSzH.print(os << here, "sock_wr.size.") << endl;
	os 
		<< ' ' << fmtAddress(cltHost())
		<< ' ' << fmtAddress(srvHost())
		<< ' ' << setw(7) << bitRate(stats().theSockRdSzH.stats())
		<< ' ' << setw(7) << bitRate(stats().theSockWrSzH.stats())
		<< ' ' << setw(5) << stats().theConnCnt
		<< ' ' << setw(5) << stats().theErrorCnt
		;
}

String ProbeLinkRec::fmtAddress(const NetAddr &addr) const {
	ostringstream buf1, buf2;
	buf1 << addr << ends;
	buf2 << setw(21) << buf1.str() << ends;

	const String s = buf2.str().c_str();
	streamFreeze(buf1, false);
	streamFreeze(buf2, false);

	return s;
}

// Mbits/sec
double ProbeLinkRec::bitRate(const AggrStat &s) const {
	const double dur = stats().duration().secd();
	if (dur <= 0)
		return -1;
	return s.sum()/(1024*1024/8)/dur;
}

static
OLog &operator <<(OLog &log, const ProbeLinkRec &rp) {
	log << ((int)0);
	rp.store(log);
	return log;
}

static
ILog &operator >>(ILog &log, ProbeLinkRec &rp) {
	int dummy = 0;
	log >> dummy;
	rp.load(log);
	return log;
}


/* ProbeStatMgr */

ProbeStatMgr::ProbeStatMgr() {
}

ProbeStatMgr::~ProbeStatMgr() {
	while (theRecs.count()) delete theRecs.pop();
}

// can be called many times to add entries
void ProbeStatMgr::incConfigure(Array<NetAddr*> &cltHosts, Array<NetAddr*> &srvHosts) {
	theRecs.stretch(theRecs.count() + cltHosts.count() * srvHosts.count());
	for (int c = 0; c < cltHosts.count(); ++c) {
		for (int s = 0; s < srvHosts.count(); ++s) {
			if (!find(*cltHosts[c], *srvHosts[s]))
				theRecs.append(new ProbeLinkRec(*cltHosts[c], *srvHosts[s]));
		}
	}
}

void ProbeStatMgr::exportStats(OLog &log) const {
	log << bege(lgProbeStats, lgcAll) << theRecs << ende;
}

void ProbeStatMgr::importStats(ILog &log) {
	Array<ProbeLinkRec*> recs;
	const LogEntryPx prefix = log.begEntry();
	Assert(prefix.theTag == lgProbeStats);
	log >> recs;
	log.endEntry();

	for (int i = 0; i < recs.count(); ++i)
		importStats(recs[i]);
}

void ProbeStatMgr::importStats(ProbeLinkRec *rec) {
	const NetAddr &cltHost = rec->cltHost();
	const NetAddr &srvHost = rec->srvHost();

	if (ProbeLinkStat *s = find(cltHost, srvHost))
		s->syncWith(rec->stats());
	else
		theRecs.append(rec);
}

ProbeLinkStat *ProbeStatMgr::stats(const NetAddr &cltHost, const NetAddr &srvHost) {
	if (ProbeLinkStat *s = find(cltHost, srvHost))
		return s;
	//clog << here << "traffic on unexpected link: " << cltHost << "<->" << srvHost << endl;
	theRecs.append(new ProbeLinkRec(cltHost, srvHost));
	return &theRecs.last()->stats();
}

void ProbeStatMgr::report(ostream &os) const {
	os << "#link "
		<< ' ' << setw(21) << "client_address"
		<< ' ' << setw(21) << "server_address"
		<< ' ' << setw(7) << "inMbps"
		<< ' ' << setw(7) << "outMbps"
		<< ' ' << setw(5) << "conn"
		<< ' ' << setw(5) << "err"
		<< endl;

	ProbeLinkRec sum(NetAddr("any", -1), NetAddr("any", -1));
	for (int i = 0; i < theRecs.count(); ++i) {
		ProbeLinkRec &rec = *theRecs[i];
		sum.stats() += rec.stats();
		report(os, rec, i+1);
	}
	os << endl;
	report(os, sum);
}

void ProbeStatMgr::report(ostream &os, const ProbeLinkRec &rec, int idx) const {
	if (idx > 0)
		os << setw(5) << idx << ' ';
	else
		os << setw(5) << 0 << ' ';

	rec.oneLineReport(os);
	os << endl;
}

ProbeLinkStat *ProbeStatMgr::find(const NetAddr &cltHost, const NetAddr &srvHost) {
	Assert(cltHost && srvHost);
	for (int i = 0; i < theRecs.count(); ++i) {
		if (theRecs[i]->cltHost() == cltHost && theRecs[i]->srvHost() == srvHost)
			return &theRecs[i]->stats();
	}
	return 0;
}
