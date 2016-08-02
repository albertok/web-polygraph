
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/math.h"
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Socket.h"
#include "xstd/Ssl.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "base/StatIntvlRec.h"

#include "xstd/gadgets.h"

EmbedStats TheEmbedStats;
TmSzStat StatIntvlRec::TheLiveReps;
TmSzStat StatIntvlRec::TheCustomLive;


StatIntvlRec::StatIntvlRec(): theXactCnt(0), theXactErrCnt(0),
	theXactRetrCnt(0), theUniqUrlCnt(0) {
	theSocksStat.progress(&TheProgress.socks);
	theSslStat.progress(&TheProgress.ssl);
	theFtpStat.progress(&TheProgress.ftp);
	theConnectStat.progress(&TheProgress.connect);
	theAuthingStat.progress(&TheProgress.authing);
}

void StatIntvlRec::updateProgress(const bool doIt) {
	theSocksStat.updateProgress =
	theSslStat.updateProgress =
	theFtpStat.updateProgress = 
	theConnectStat.updateProgress = 
	theAuthingStat.updateProgress =
		doIt;
}

void StatIntvlRec::restart() {
	thePopulusLvl.restart();
	theWaitLvl.restart();
	theXactLvl.restart();
	theOpenLvl.restart();
	theEstbLvl.restart();
	theIdleLvl.restart();
	theBaseLvl.restart();

	theConnLifeTm.reset();
	theConnUseCnt.reset();
	theConnPipelineDepth.reset();

	theIdealHR.reset();
	theRealHR.reset();
	theChbR.reset();

	theFill.reset();
	theRediredReq.reset();
	theRepToRedir.reset();
	theIms.reset();
	theReload.reset();
	theRange.reset();
	theHead.reset();
	thePost.reset();
	thePut.reset();
	theAbort.reset();

	thePage.reset();

	theCustom.reset();

	theXactCnt = theXactErrCnt = theXactRetrCnt = theUniqUrlCnt = 0;

	theIcpStat.reset();
	theSocksStat.restart();
	theSslStat.restart();
	theFtpStat.restart();
	theConnectStat.restart();
	theAuthingStat.restart();

	theContinueMsg.reset();

	theProxyValidations.reset();

	theAuth.reset();
	theAuthNone.reset();
	theTunneled.reset();
	theBaseline.reset();

	theRepContType.reset();
	theReqContType.reset();

	theDuration = Time();
}

OLog &StatIntvlRec::store(OLog &log) const {
	Assert(theDuration >= 0);
	return log
		<< thePopulusLvl
		<< theWaitLvl << theXactLvl << theOpenLvl << theEstbLvl << theIdleLvl
		<< theBaseLvl
		<< theConnLifeTm << theConnUseCnt
		<< theIdealHR << theRealHR << theChbR
		<< theFill 
		<< theRediredReq << theRepToRedir << theIms << theReload << theRange
		<< theHead << thePost << thePut << theAbort
		<< thePage
		<< theCustom
		<< theXactCnt
		<< theXactErrCnt
		<< theXactRetrCnt
		<< theIcpStat
		<< theSocksStat
		<< theSslStat
		<< theDuration
		<< theConnPipelineDepth
		<< theContinueMsg
		<< theProxyValidations
		<< theAuth
		<< theAuthNone
		<< theTunneled
		<< theUniqUrlCnt
		<< theFtpStat
		<< theRepContType
		<< theReqContType
		<< theConnectStat
		<< theAuthingStat
		<< theBaseline
		;
}

ILog &StatIntvlRec::load(ILog &log) {
	return log
		>> thePopulusLvl
		>> theWaitLvl >> theXactLvl >> theOpenLvl >> theEstbLvl >> theIdleLvl
		>> theBaseLvl
		>> theConnLifeTm >> theConnUseCnt
		>> theIdealHR >> theRealHR >> theChbR
		>> theFill 
		>> theRediredReq >> theRepToRedir >> theIms >> theReload >> theRange
		>> theHead >> thePost >> thePut >> theAbort
		>> thePage
		>> theCustom
		>> theXactCnt
		>> theXactErrCnt
		>> theXactRetrCnt
		>> theIcpStat
		>> theSocksStat
		>> theSslStat
		>> theDuration
		>> theConnPipelineDepth
		>> theContinueMsg
		>> theProxyValidations
		>> theAuth
		>> theAuthNone
		>> theTunneled
		>> theUniqUrlCnt
		>> theFtpStat
		>> theRepContType
		>> theReqContType
		>> theConnectStat
		>> theAuthingStat
		>> theBaseline
		;
}

bool StatIntvlRec::sane() const {
	return
		thePopulusLvl.sane() &&
		theWaitLvl.sane() && theXactLvl.sane() &&
		theOpenLvl.sane() && theEstbLvl.sane() && theIdleLvl.sane() &&
		theBaseLvl.sane() &&
		theConnLifeTm.sane() && theConnUseCnt.sane() &&
		theConnPipelineDepth.sane() && 
		theRediredReq.sane() && theReload.sane() && theRange.sane() &&
		theHead.sane() && thePost.sane() && thePut.sane() &&
		theAbort.sane() &&
		thePage.sane() &&
		theCustom.sane() &&
		theIcpStat.sane() && theSocksStat.sane() &&
		theSslStat.sane() && theFtpStat.sane() &&
		theConnectStat.sane() &&
		theAuthingStat.sane() &&
		theXactCnt >= 0 && theXactErrCnt >= 0 && 
		theXactRetrCnt >= 0 &&
		theContinueMsg.sane() &&
		theProxyValidations.sane() &&
		theAuth.sane() &&
		theAuthNone.sane() &&
		theTunneled.sane() &&
		theBaseline.sane() &&
		theRepContType.sane() && theReqContType.sane() &&
		theUniqUrlCnt >= 0 &&
		theDuration >= 0;
}

Counter StatIntvlRec::xactCnt() const {
	return theBaseline.repSize.stats().count() + theXactErrCnt;
}

BigSize StatIntvlRec::totFillSize() const {
	return BigSize::Byted(theFill.size().sum());
}

Counter StatIntvlRec::totFillCount() const {
	return theFill.size().count();
}

double StatIntvlRec::errRatio() const {
	return Ratio(theXactErrCnt, xactCnt());
}

double StatIntvlRec::errPercent() const {
	return Percent(theXactErrCnt, xactCnt());
}

double StatIntvlRec::recurrenceRatio() const {
	const Counter total = xactCnt();
	return Percent(total - theUniqUrlCnt, total);
}

// XXX: we should have Rate type, not double
double StatIntvlRec::reqRate() const {
	return theDuration > 0 ?
		Ratio(theBaseLvl.incCnt(), theDuration.secd()) : -1;
}

double StatIntvlRec::repRate() const {
	return theDuration > 0 ?
		Ratio(theBaseLvl.decCnt(), theDuration.secd()) : -1;
}

double StatIntvlRec::reqBwidth() const {
	return theDuration > 0 ?
		Ratio(theBaseline.reqSize.stats().sum(), theDuration.secd()) : -1;
}

double StatIntvlRec::repBwidth() const {
	return theDuration > 0 ?
		Ratio(theBaseline.repSize.stats().sum(), theDuration.secd()) : -1;
}

AggrStat StatIntvlRec::repTime() const {
	return theBaseline.duration.stats();
}

// TODO: Or should this report the last response size instead of the compound?
AggrStat StatIntvlRec::repSize() const {
	return theBaseline.repSize.stats();
}

TmSzStat StatIntvlRec::reps() const {
	TmSzStat reps;
	reps += theRealHR.xacts();
	reps += theRediredReq;
	reps += theRepToRedir;
	reps += theIms;
	reps += theReload;
	reps += theRange;
	reps += theHead;
	reps += thePost;
	reps += thePut;
	reps += theConnectStat.doneXacts().xacts();
	reps += theFtpStat.doneXacts().xacts();
	theAuth.authIngAll(reps);
	return reps;
}

TmSzStat StatIntvlRec::projectedReps() const {
	return reps() + TheLiveReps;
}

TmSzStat StatIntvlRec::customProjectedReps() const {
	return theCustom + TheCustomLive;
}

void StatIntvlRec::keepLevels(const StatIntvlRec &prevIntvl) {
	thePopulusLvl.keepLevel(prevIntvl.thePopulusLvl);
	theWaitLvl.keepLevel(prevIntvl.theWaitLvl);
	theXactLvl.keepLevel(prevIntvl.theXactLvl);
	theOpenLvl.keepLevel(prevIntvl.theOpenLvl);
	theEstbLvl.keepLevel(prevIntvl.theEstbLvl);
	theIdleLvl.keepLevel(prevIntvl.theIdleLvl);
	theBaseLvl.keepLevel(prevIntvl.theBaseLvl);
	theSocksStat.keepLevel(prevIntvl.theSocksStat);
	theSslStat.keepLevel(prevIntvl.theSslStat);
	theFtpStat.keepLevel(prevIntvl.theFtpStat);
	theConnectStat.keepLevel(prevIntvl.theConnectStat);
	theAuthingStat.keepLevel(prevIntvl.theAuthingStat);
}

void StatIntvlRec::concat(const StatIntvlRec &s) {
	if (theDuration >= 0)
		theDuration += s.theDuration;

	thePopulusLvl.concat(s.thePopulusLvl);
	theWaitLvl.concat(s.theWaitLvl);
	theXactLvl.concat(s.theXactLvl);
	theOpenLvl.concat(s.theOpenLvl);
	theEstbLvl.concat(s.theEstbLvl);
	theIdleLvl.concat(s.theIdleLvl);
	theBaseLvl.concat(s.theBaseLvl);
	theSocksStat.concat(s.theSocksStat);
	theSslStat.concat(s.theSslStat);
	theFtpStat.concat(s.theFtpStat);
	theConnectStat.concat(s.theConnectStat);
	theAuthingStat.concat(s.theAuthingStat);

	join(s);
}

void StatIntvlRec::merge(const StatIntvlRec &s) {
	if (theDuration >= 0)
		theDuration = Max(theDuration, s.theDuration);
	thePopulusLvl.merge(s.thePopulusLvl);
	theWaitLvl.merge(s.theWaitLvl);
	theXactLvl.merge(s.theXactLvl);
	theOpenLvl.merge(s.theOpenLvl);
	theEstbLvl.merge(s.theEstbLvl);
	theIdleLvl.merge(s.theIdleLvl);
	theBaseLvl.merge(s.theBaseLvl);
	theSocksStat.merge(s.theSocksStat);
	theSslStat.merge(s.theSslStat);
	theFtpStat.merge(s.theFtpStat);
	theConnectStat.merge(s.theConnectStat);
	theAuthingStat.merge(s.theAuthingStat);

	join(s);
}

void StatIntvlRec::join(const StatIntvlRec &s) {
	Assert(s.theDuration >= 0);
	if (theDuration < 0)
		theDuration = s.theDuration;

	theConnLifeTm += s.theConnLifeTm;
	theConnUseCnt += s.theConnUseCnt;
	theConnPipelineDepth += s.theConnPipelineDepth;

	theIdealHR += s.theIdealHR;
	theRealHR += s.theRealHR;
	theChbR += s.theChbR;

	theFill += s.theFill;
	theRediredReq += s.theRediredReq;
	theRepToRedir += s.theRepToRedir;
	theIms += s.theIms;
	theReload += s.theReload;
	theRange += s.theRange;
	theHead += s.theHead;
	thePost += s.thePost;
	thePut += s.thePut;
	theAbort += s.theAbort;

	thePage += s.thePage;

	theCustom += s.theCustom;

	theXactCnt += s.theXactCnt;
	theXactErrCnt += s.theXactErrCnt;
	theXactRetrCnt += s.theXactRetrCnt;

	theIcpStat += s.theIcpStat;

	theContinueMsg += s.theContinueMsg;

	theProxyValidations += s.theProxyValidations;

	theAuth += s.theAuth;
	theAuthNone += s.theAuthNone;
	theTunneled += s.theTunneled;

	theRepContType.add(s.theRepContType);
	theReqContType.add(s.theReqContType);

	theBaseline += s.theBaseline;

	theUniqUrlCnt += s.theUniqUrlCnt;
}

ostream &StatIntvlRec::print(ostream &os, const String &pfx) const {
	os << pfx << "baseline.req.rate:\t " << reqRate() << endl;
	os << pfx << "baseline.rep.rate:\t " << repRate() << endl;
	theBaseline.print(os, pfx + "baseline.");

	reps().print(os, pfx + "rep.");

	theRealHR.xacts().print(os, pfx + "basic.");
	theIdealHR.print(os, "hit", "miss", pfx + "offered.");
	theRealHR.print(os, "hit", "miss", pfx);
	theChbR.print(os, "cachable", "uncachable", pfx);
	theFill.print(os, pfx + "fill.");

	theRediredReq.print(os, pfx + "redired_req.");
	theRepToRedir.print(os, pfx + "rep_to_redir.");
	theIms.print(os, pfx + "ims.");
	theReload.print(os, pfx + "reload.");
	theRange.print(os, pfx + "range.");
	theHead.print(os, pfx + "head.");
	thePost.print(os, pfx + "post.");
	thePut.print(os, pfx + "put.");
	// theConnect.print(os, pfx + "connect."); see theConnectStat
	theAbort.print(os, pfx + "abort.");

	thePage.print(os, pfx + "page.");

	theRepContType.print(os, pfx + "cont_type.");
	theReqContType.print(os, pfx + "req_cont_type.");

	theCustom.print(os, pfx + "custom.rep.");

	theXactLvl.print(os, pfx + "xact.");

	os << pfx << "ok_xact.count: \t " << theXactCnt << endl;
	os << pfx << "err_xact.ratio:\t " << errPercent() << endl;
	os << pfx << "err_xact.count:\t " << theXactErrCnt << endl;
	os << pfx << "retr_xact.count:\t " << theXactRetrCnt << endl;

	thePopulusLvl.print(os, pfx + "populus.");
	theWaitLvl.print(os, pfx + "wait.");
	theOpenLvl.print(os, pfx + "conn.open.");
	theEstbLvl.print(os, pfx + "conn.estb.");
	theBaseLvl.print(os, pfx + "baseline.");
	theConnLifeTm.print(os, pfx + "conn.ttl.");
	theConnUseCnt.print(os, pfx + "conn.use.");
	theConnPipelineDepth.print(os, pfx + "conn.pipeline.depth.");

	theIcpStat.print(os, pfx + "icp.", theDuration);
	theSocksStat.print(os, pfx + "socks.", theDuration);
	theSslStat.print(os, pfx + "ssl.", theDuration);
	theFtpStat.print(os, pfx + "ftp.", theDuration);
	theConnectStat.print(os, pfx + "connect.", theDuration);
	theAuthingStat.print(os, pfx + "authenticating.", theDuration);

	theContinueMsg.print(os, pfx + "100_continue.");

	theProxyValidations.print(os, pfx + "proxy_validations.");

	theAuth.print(os, pfx + "auth.");
	theAuthNone.print(os, pfx + "auth.none.");
	theTunneled.print(os, pfx + "tunneled.");

	os << pfx << "url.recurrence.ratio:\t " << recurrenceRatio() << endl;
	os << pfx << "url.unique.count:\t " << theUniqUrlCnt << endl;

	os << pfx << "duration:\t " << theDuration.secd() << endl;

	return os;
}

void StatIntvlRec::linePrintProtos(ostream &os, int offset, bool includeLevels) const {
	theSocksStat.linePrint(os, offset, theDuration, includeLevels);
	theSslStat.linePrint(os, offset, theDuration, includeLevels);
	theFtpStat.linePrint(os, offset, theDuration, includeLevels);
	theConnectStat.linePrint(os, offset, theDuration, includeLevels);
	theAuthingStat.linePrint(os, offset, theDuration, includeLevels);
}

void StatIntvlRec::linePrintAll(ostream &os, bool includeLevels) const {
	// XXX: hack to align proto lines
	const streampos offset = os.tellp();

	os
		<< ' ' << setw(6) << TheProgress.xacts()
		<< ' ' << setw(6) << repRate()
		<< ' ' << setw(6) << (int)rint(repTime().mean())
		<< ' ' << setw(6) << theRealHR.dhp()
		<< ' ' << setw(3) << theXactErrCnt
		;

	if (includeLevels)
		os << ' ' << setw(4) << Socket::Level();

	// hack: increase the offset to compensate for endc
	const streampos extra = 8;
	linePrintProtos(os, offset + extra, includeLevels);
}
