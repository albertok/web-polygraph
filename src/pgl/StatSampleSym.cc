
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "pgl/pgl.h"

#include "xstd/String.h"
#include "xparser/SynSymTbl.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/StatSampleSym.h"


String StatSampleSym::TheType = "StatSample";

StatSampleSym::StatSampleSym(): ExpressionSym(TheType) {
}

StatSampleSym::StatSampleSym(const Rec &aRec): 
	ExpressionSym(TheType), theRec(aRec) {
}

StatSampleSym::~StatSampleSym() {
	while (theSymGarbage.count()) delete theSymGarbage.pop();
}

bool StatSampleSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *StatSampleSym::dupe(const String &type) const {
	if (isA(type))
		return new StatSampleSym(theRec);
	return ExpressionSym::dupe(type);
}

bool StatSampleSym::memberMatch(const String &prefix, const char *name, const char **tail) const {
	if (tail && prefix.prefixOf(name)) { // prefix match
		*tail = name + prefix.len();
		return true;
	} else
	if (prefix == name) { // full match
		return true;
	}
	return false;
}

SynSymTblItem **StatSampleSym::memberItem(const String &name) {
	static const String nameAllResponses = "rep.";
	static const String nameCustomReps = "custom.rep.";
	static const String nameBasicResponses = "basic.";
	static const String nameOffered = "offered.";
	static const String nameReal = "real.";
	static const String nameHit = "hit.";
	static const String nameMiss = "miss.";
	static const String nameCachable = "cachable.";
	static const String nameYes = "yes.";
	static const String nameNo = "no.";
	static const String nameRediredReq = "redired_req.";
	static const String nameRepToRedir = "rep_to_redir.";
	static const String nameFill = "fill.";
	static const String nameIms = "ims.";
	static const String nameReload = "reload.";
	static const String nameRange = "range.";
	static const String nameHead = "head.";
	static const String namePost = "post.";
	static const String namePut = "put.";
	static const String nameAbort = "abort.";
	static const String nameXact = "xact.";
	static const String namePopulus = "populus.";
	static const String nameWait = "wait.";
	static const String nameConnOpen = "conn.open.";
	static const String nameConnEstb = "conn.estb.";
	static const String nameConnIdle = "conn.idle.";
	static const String nameConnTtl = "conn.ttl.";
	static const String nameConnUse = "conn.use.";
	//static const String nameIcp = "icp.";
	static const String nameSocks = "socks.";
	static const String nameSsl = "ssl.";
	static const String nameFtp = "ftp.";
	static const String nameConnect = "connect.";
	static const String nameAuthenticating = "authenticating.";
	static const String nameBaselineRptm = "baseline.rptm.";
	static const String namePage = "page.";
	static const String nameOkXactCount = "ok_xact.count";
	static const String nameErrXactRatio = "err_xact.ratio";
	static const String nameErrXactCount = "err_xact.count";
	static const String nameRetrXactCount = "retr_xact.count";
	static const String nameDuration = "duration";
	static const String nameAuthNone = "auth.none.";
	static const String nameAuthIng = "auth.ing.";
	static const String nameAuthEd = "auth.ed.";
	static const String nameTunneled = "tunneled.";
	static const String nameLiveRep = "live.rep.";
	static const String nameProjectedRep = "projected.rep.";
	static const String nameCustomLiveReps = "custom.live.rep.";
	static const String nameCustomProjectedReps = "custom.projected.rep.";

	SynSym *s = 0;
	const char *key = name.cstr();
	const char *subKey = 0;

	if (!s && name == "baseline.req.rate")
		s = new NumSym(theRec.reqRate());

	if (!s && name == "baseline.rep.rate")
		s = new NumSym(theRec.repRate());

	if (!s && memberMatch(nameAllResponses, key, &subKey))
		s = memberTmSz(subKey, theRec.reps());

	if (!s && memberMatch(nameCustomReps, key, &subKey))
		s = memberTmSz(subKey, theRec.theCustom);

	if (!s && memberMatch(nameBasicResponses, key, &subKey))
		s = memberTmSz(subKey, theRec.theRealHR.xacts());

	if (!s && memberMatch(nameOffered, key, &subKey))
		s = memberHR(subKey, theRec.theIdealHR, nameHit, nameMiss);

	if (!s && memberMatch(nameReal, key, &subKey))
		s = memberHR(subKey, theRec.theRealHR, nameHit, nameMiss);

	if (!s && memberMatch(nameCachable, key, &subKey))
		s = memberHR(subKey, theRec.theChbR, nameYes, nameNo);

	if (!s && memberMatch(nameFill, key, &subKey))
		s = memberTmSz(subKey, theRec.theFill);

	if (!s && memberMatch(nameRediredReq, key, &subKey))
		s = memberTmSz(subKey, theRec.theRediredReq);

	if (!s && memberMatch(nameRepToRedir, key, &subKey))
		s = memberTmSz(subKey, theRec.theRepToRedir);

	if (!s && memberMatch(nameIms, key, &subKey))
		s = memberTmSz(subKey, theRec.theIms);

	if (!s && memberMatch(nameReload, key, &subKey))
		s = memberTmSz(subKey, theRec.theReload);

	if (!s && memberMatch(nameRange, key, &subKey))
		s = memberTmSz(subKey, theRec.theRange);

	if (!s && memberMatch(nameHead, key, &subKey))
		s = memberTmSz(subKey, theRec.theHead);

	if (!s && memberMatch(namePost, key, &subKey))
		s = memberTmSz(subKey, theRec.thePost);

	if (!s && memberMatch(namePut, key, &subKey))
		s = memberTmSz(subKey, theRec.thePut);

	if (!s && memberMatch(nameAbort, key, &subKey))
		s = memberTmSz(subKey, theRec.theAbort);

	if (!s && memberMatch(namePage, key, &subKey))
		s = memberTmSz(subKey, theRec.thePage);

	if (!s && memberMatch(nameXact, key, &subKey))
		s = memberLevel(subKey, theRec.theXactLvl);

	if (!s && memberMatch(namePopulus, key, &subKey))
		s = memberLevel(subKey, theRec.thePopulusLvl);

	if (!s && memberMatch(nameWait, key, &subKey))
		s = memberLevel(subKey, theRec.theWaitLvl);

	if (!s && memberMatch(nameConnOpen, key, &subKey))
		s = memberLevel(subKey, theRec.theOpenLvl);

	if (!s && memberMatch(nameConnEstb, key, &subKey))
		s = memberLevel(subKey, theRec.theEstbLvl);

	if (!s && memberMatch(nameConnIdle, key, &subKey))
		s = memberLevel(subKey, theRec.theIdleLvl);

	if (!s && memberMatch(nameConnTtl, key, &subKey))
		s = memberAggr(subKey, theRec.theConnLifeTm, TimeSym::TheType);

	if (!s && memberMatch(nameConnUse, key, &subKey))
		s = memberAggr(subKey, theRec.theConnUseCnt, IntSym::TheType);

	/*
	if (!s && memberMatch(nameIcp, key, &subKey))
		s = memberIcp(subKey, theRec.theIcpStat);
	*/

	if (!s && memberMatch(nameSocks, key, &subKey))
		s = memberProto(subKey, theRec.theSocksStat);

	if (!s && memberMatch(nameSsl, key, &subKey))
		s = memberProto(subKey, theRec.theSslStat);

	if (!s && memberMatch(nameFtp, key, &subKey))
		s = memberProto(subKey, theRec.theFtpStat);

	if (!s && memberMatch(nameConnect, key, &subKey))
		s = memberProto(subKey, theRec.theConnectStat);

	if (!s && memberMatch(nameAuthenticating, key, &subKey))
		s = memberProto(subKey, theRec.theAuthingStat);

	// TODO: Add access to other baseline details.
	if (!s && memberMatch(nameBaselineRptm, key, &subKey))
		s = memberAggr(subKey, theRec.theBaseline.duration.stats(), TimeSym::TheType);

	if (!s && memberMatch(nameOkXactCount, key, &subKey))
		s = new NumSym(theRec.theXactCnt);

	if (!s && memberMatch(nameErrXactRatio, key, &subKey))
		s = new NumSym(theRec.errPercent());

	if (!s && memberMatch(nameErrXactCount, key, &subKey))
		s = new NumSym(theRec.theXactErrCnt);

	if (!s && memberMatch(nameRetrXactCount, key, &subKey))
		s = new NumSym(theRec.theXactRetrCnt);

	if (!s && memberMatch(nameDuration, key, &subKey))
		s = new TimeSym(theRec.theDuration);

	if (!s && memberMatch(nameAuthNone, key, &subKey))
		s = memberTmSz(subKey, theRec.theAuthNone);

	if (!s && memberMatch(nameAuthIng, key, &subKey)) {
		TmSzStat st;
		theRec.theAuth.authIngAll(st);
		s = memberTmSz(subKey, st);
	}

	if (!s && memberMatch(nameAuthEd, key, &subKey)) {
		TmSzStat st;
		theRec.theAuth.authEdAll(st);
		s = memberTmSz(subKey, st);
	}

	if (!s && memberMatch(nameTunneled, key, &subKey))
		s = memberTmSz(subKey, theRec.theTunneled);

	if (!s && memberMatch(nameLiveRep, key, &subKey))
		s = memberTmSz(subKey, StatIntvlRec::TheLiveReps);

	if (!s && memberMatch(nameProjectedRep, key, &subKey))
		s = memberTmSz(subKey, theRec.projectedReps());

	if (!s && memberMatch(nameCustomLiveReps, key, &subKey))
		s = memberTmSz(subKey, StatIntvlRec::TheCustomLive);

	if (!s && memberMatch(nameCustomProjectedReps, key, &subKey))
		s = memberTmSz(subKey, theRec.customProjectedReps());

	// TODO: Add access to preliminary stats when we have them

	if (!s)
		return 0;

	s->loc(loc());

	SynSymTblItem *i = new SynSymTblItem(s->type(), name);
	i->ctx(0); // read-only
	i->sym(s);
	i->loc(s->loc());
	i->use();

	theSymGarbage.push(i);
	return &theSymGarbage.last();
}

SynSym *StatSampleSym::memberLevel(const char *key, const LevelStat &stats) const {
	static const String nameStarted = "started";
	static const String nameFinished = "finished";
	static const String nameLevelMean = "level.mean";
	static const String nameLevelLast = "level.last";

	if (nameStarted == key)
		return new IntSym(stats.incCnt());

	if (nameFinished == key)
		return new IntSym(stats.decCnt());

	if (nameLevelMean == key)
		return new NumSym(stats.mean());

	if (nameLevelLast == key)
		return new IntSym(stats.level());

	return 0;
}

SynSym *StatSampleSym::memberHR(const char *key, const HRStat &stats, const String &nameHit, const String &nameMiss) const {
	static const String nameRatioObj = "ratio.obj";
	static const String nameRatioByte = "ratio.byte";

	if (nameRatioObj == key)
		return new NumSym(stats.dhp());

	if (nameRatioByte == key)
		return new NumSym(stats.bhp());
	
	const char *subKey = 0;

	if (memberMatch(nameHit, key, &subKey))
		return memberTmSz(subKey, stats.hits());

	if (memberMatch(nameMiss, key, &subKey))
		return memberTmSz(subKey, stats.misses());

	return 0;
}

SynSym *StatSampleSym::memberTmSz(const char *key, const TmSzStat &stats) const {
	static const String nameRptm = "rptm.";
	static const String nameSize = "size.";

	const char *subKey = 0;

	if (memberMatch(nameRptm, key, &subKey))
		return memberAggr(subKey, stats.time(), TimeSym::TheType);

	if (memberMatch(nameSize, key, &subKey))
		return memberAggr(subKey, stats.size(), SizeSym::TheType);

	return 0;
}

SynSym *StatSampleSym::memberAggr(const char *key, const AggrStat &stats, const String &stype) const {
	static const String nameCount = "count";
	static const String nameMean = "mean";
	static const String nameMin = "min";
	static const String nameMax = "max";
	static const String nameStd = "std_dev";
	static const String nameRel = "rel_dev";
	static const String nameSum = "sum";

	if (memberMatch(nameCount, key, 0))
		return new IntSym(stats.count());

	if (memberMatch(nameMean, key, 0))
		return memberSym(stats.mean(), stype);

	if (memberMatch(nameMin, key, 0))
		return memberSym(stats.min(), stype);

	if (memberMatch(nameMax, key, 0))
		return memberSym(stats.max(), stype);

	if (memberMatch(nameStd, key, 0))
		return new NumSym(stats.stdDev());

	if (memberMatch(nameRel, key, 0))
		return new NumSym(stats.relDevp());

	if (memberMatch(nameSum, key, 0))
		return memberSym(stats.sum(), stype);

	return 0;
}

SynSym *StatSampleSym::memberSym(double value, const String &stype) const {
	if (stype == IntSym::TheType)
		return new IntSym((int)value);

	if (stype == NumSym::TheType)
		return new NumSym(value);

	if (stype == TimeSym::TheType)
		return new TimeSym(Time::Secd(value/1000.));

	if (stype == SizeSym::TheType)
		return new SizeSym(BigSize::Byted(value));

	// unknown member type
	Assert(false);
	return 0;
}

SynSym *StatSampleSym::memberProto(const char *key, const ProtoIntvlStat &stats) const {
	static const String nameXact = "xact.";
	static const String nameConn = "conn.";
	static const String nameErrXactCount = "err_xact.count";
	static const String nameHit = "hit.";
	static const String nameMiss = "miss.";

	SynSym *s;
	const char *subKey;
	if (memberMatch(nameXact, key, &subKey)) {
		s = memberHR(subKey, stats.doneXacts(), nameHit, nameMiss);
		if (!s)
			s = memberLevel(subKey, stats.xactLevel());
	} else
	if (memberMatch(nameConn, key, &subKey))
		s = memberLevel(subKey, stats.connLevel());
	else
	if (memberMatch(nameErrXactCount, key, &subKey))
		s = new NumSym(stats.errXacts());
	else
		s = 0;

	return s;
}

ostream &StatSampleSym::print(ostream &os, const String &pfx) const {
	// XXX: this is wrong, output must be valid PGL instead
	return theRec.print(os, pfx);
}
