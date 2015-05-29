
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "base/AnyToString.h"
#include "base/ErrorRec.h"
#include "base/ErrorStat.h"
#include "base/ProtoIntvlStat.h"
#include "base/polyLogCats.h"
#include "runtime/httpHdrs.h"
#include "runtime/HttpDate.h"
#include "xml/XmlAttr.h"
#include "xml/XmlParagraph.h"
#include "xml/XmlText.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/HistogramFigure.h"
#include "loganalyzers/RptmHistFig.h"
#include "loganalyzers/SizeHistFig.h"
#include "loganalyzers/PointTraceFig.h"
#include "loganalyzers/RptmTraceFig.h"
#include "loganalyzers/LevelTraceFig.h"
#include "loganalyzers/LoadTraceFig.h"
#include "loganalyzers/ScatteredFig.h"
#include "loganalyzers/InfoScopes.h"
#include "loganalyzers/Stex.h"
#include "loganalyzers/PointStex.h"
#include "loganalyzers/HistStex.h"
#include "loganalyzers/LevelStex.h"
#include "loganalyzers/LoadStexes.h"
#include "loganalyzers/PhaseTrace.h"
#include "loganalyzers/TestInfo.h"
#include "loganalyzers/ProcInfo.h"
#include "loganalyzers/SideInfo.h"
#include "loganalyzers/StatTable.h"

Stex *SideInfo::TheAllReps = 0;
Stex *SideInfo::TheAllReqs = 0;
Stex *SideInfo::TheUsefulProxyValidation = 0;
Array<Stex*> SideInfo::TheReplyStex;
Array<Stex*> SideInfo::TheRequestStex;
Array<Stex*> SideInfo::TheCompoundReplyStex;
Array<Stex*> SideInfo::TheCompoundRequestStex;
Array<Stex*> SideInfo::TheAuthStex;
Array<Stex*> SideInfo::TheReplyStatusStex;

static XmlAttr algnLeft("align", "left");
static XmlAttr algnRight("align", "right");


SideInfo::SideInfo(int aLogCat): theLogCat(aLogCat), theTest(0) {
	Assert(theLogCat == lgcCltSide || theLogCat == lgcSrvSide);
	theScope.name("all phases");
	theScope.addSide(name());
}

SideInfo::~SideInfo() {
	while (theProcs.count()) {
		theProcs.last()->side(0);
		delete theProcs.pop();
	}
	
	while (thePhases.count()) delete thePhases.pop();
}

void SideInfo::test(TestInfo *t) {
	Assert(!theTest ^ !t);
	theTest = t;
}

int SideInfo::logCat() const {
	return theLogCat;
}

const String &SideInfo::name() const {
	static String cltName = "client";
	static String srvName = "server";
	return theLogCat == lgcCltSide ? cltName : srvName;
}

const String &SideInfo::otherName() const {
	static String cltName = "client";
	static String srvName = "server";
	return theLogCat == lgcCltSide ? srvName : cltName;
}

const String &SideInfo::benchmarkVersion() const {
	return theBenchmarkVersion;
}

const String &SideInfo::pglCfg() const {
	return thePglCfg;
}

Time SideInfo::startTime() const {
	return theStartTime;
}

int SideInfo::scopes(InfoScopes &res) const {
	res.add(execScope());
	res.add(theScope);

	for (int p = 0; p < thePhases.count(); ++p) {
		const String &pname = thePhases[p]->name();
		Scope *scope = new Scope(theScope.onePhase(pname));
		scope->name(pname);
		res.absorb(scope);
	}

	return res.count();
}

const InfoScope &SideInfo::execScope() const {
	Assert(theTest);
	if (!theExecScope) {
		theExecScope = theTest->execScope().oneSide(name());
		theExecScope.name("baseline");
	}
	return theExecScope;
}

const StatPhaseRec &SideInfo::execScopeStats() const {
	return theExecScopePhase.stats();
}

int SideInfo::repCount(const Scope &scope) const {
	int count = 0;
	for (int i = 0; i < theProcs.count(); ++i) {
		count += theProcs[i]->repCount(scope);
	}
	return count;
}

int SideInfo::hitCount(const Scope &scope) const {
	int count = 0;
	for (int i = 0; i < theProcs.count(); ++i) {
		count += theProcs[i]->hitCount(scope);
	}
	return count;
}

int SideInfo::offeredHitCount(const Scope &scope) const {
	int count = 0;
	for (int i = 0; i < theProcs.count(); ++i) {
		count += theProcs[i]->offeredHitCount(scope);
	}
	return count;
}

int SideInfo::uselessProxyValidationCount(const Scope &scope) const {
	int count = 0;
	for (int i = 0; i < theProcs.count(); ++i) {
		count += theProcs[i]->uselessProxyValidationCount(scope);
	}
	return count;
}

BigSize SideInfo::repVolume(const Scope &scope) const {
	BigSize volume = 0;
	for (int i = 0; i < theProcs.count(); ++i) {
		volume += theProcs[i]->repVolume(scope);
	}
	return volume;
}

BigSize SideInfo::hitVolume(const Scope &scope) const {
	BigSize volume = 0;
	for (int i = 0; i < theProcs.count(); ++i) {
		volume += theProcs[i]->hitVolume(scope);
	}
	return volume;
}

BigSize SideInfo::offeredHitVolume(const Scope &scope) const {
	BigSize volume = 0;
	for (int i = 0; i < theProcs.count(); ++i) {
		volume += theProcs[i]->offeredHitVolume(scope);
	}
	return volume;
}

BigSize SideInfo::uselessProxyValidationVolume(const Scope &scope) const {
	BigSize volume = 0;
	for (int i = 0; i < theProcs.count(); ++i) {
		volume += theProcs[i]->uselessProxyValidationVolume(scope);
	}
	return volume;
}

AggrStat SideInfo::lastReqByteWritten(const Scope &scope) const {
	AggrStat stat;
	for (int i = 0; i < theProcs.count(); ++i) {
		stat += theProcs[i]->lastReqByteWritten(scope);
	}
	return stat;
}

AggrStat SideInfo::lastReqByteRead(const Scope &scope) const {
	AggrStat stat;
	for (int i = 0; i < theProcs.count(); ++i) {
		stat += theProcs[i]->lastReqByteRead(scope);
	}
	return stat;
}

AggrStat SideInfo::firstRespByteWritten(const Scope &scope) const {
	AggrStat stat;
	for (int i = 0; i < theProcs.count(); ++i) {
		stat += theProcs[i]->firstRespByteWritten(scope);
	}
	return stat;
}

AggrStat SideInfo::firstRespByteRead(const Scope &scope) const {
	AggrStat stat;
	for (int i = 0; i < theProcs.count(); ++i) {
		stat += theProcs[i]->firstRespByteRead(scope);
	}
	return stat;
}

void SideInfo::add(ProcInfo *p) {
	Assert(p);
	p->side(this);
	theProcs.append(p);

	// sync phases
	thePhases.stretch(p->phaseCount());
	for (int i = 0; i < p->phaseCount(); ++i)
		addPhase(p->phase(i));
}

void SideInfo::addPhase(const PhaseInfo &procPhase) {
	const String &name = procPhase.name();
	PhaseInfo *accPhase = findPhase(name);
	if (!accPhase) {
		accPhase = new PhaseInfo();
		thePhases.append(accPhase);
		theScope.addPhase(name);
	}
	accPhase->merge(procPhase);
}

ProcInfo &SideInfo::proc(int idx) {
	Assert(0 <= idx && idx < theProcs.count());
	return *theProcs[idx];
}

int SideInfo::procCount() const {
	return theProcs.count();
}

const PhaseInfo &SideInfo::phase(const Scope &scope) const {
	if (scope.phases().count() == 1)
		return phase(*scope.phases().last());

	if (scope.phases().count() == thePhases.count())
		return theAllPhasesPhase;

	return theExecScopePhase; // what else can it be?
}

const PhaseInfo &SideInfo::phase(const String &name) const {
	const PhaseInfo *p = findPhase(name);
	Assert(p);
	return *p;
}

const PhaseInfo *SideInfo::findPhase(const String &name) const {
	for (int i = 0; i < thePhases.count(); ++i) {
		if (thePhases[i]->name() == name)
			return thePhases[i];
	}
	return 0;
}

PhaseInfo *SideInfo::findPhase(const String &name) {
	for (int i = 0; i < thePhases.count(); ++i) {
		if (thePhases[i]->name() == name)
			return thePhases[i];
	}
	return 0;
}

const PhaseInfo &SideInfo::phase(int idx) const {
	Assert(0 <= idx && idx < thePhases.count());
	return *thePhases[idx];
}

int SideInfo::phaseCount() const {
	return thePhases.count();
}

void SideInfo::checkCommonPglCfg() {
	Assert(!thePglCfg);
	if (procCount()) {
		const ProcInfo &p = proc(0);
		bool mismatch = false;
		for (int i = 1; i < procCount(); ++i) {
			if (p.pglCfg() != proc(i).pglCfg()) {
				mismatch = true;
				cerr << "PGL configuration in " << p.name() 
					<< " differs from the one in " << proc(i).name()
					<< endl;
			}
		}
		if (!mismatch)
			thePglCfg = p.pglCfg();
	}
}

void SideInfo::checkCommonBenchmarkVersion() {
	Assert(!theBenchmarkVersion);
	if (procCount()) {
		const ProcInfo &p = proc(0);
		bool mismatch = false;
		for (int i = 1; i < procCount(); ++i) {
			if (p.benchmarkVersion() != proc(i).benchmarkVersion()) {
				mismatch = true;
				cerr << "benchmark version in " << p.name() 
					<< " differs from the one in " << proc(i).name()
					<< endl;
			}
		}
		if (!mismatch)
			theBenchmarkVersion = p.benchmarkVersion();
	}
}

void SideInfo::checkCommonStartTime() {
	Time firstTime, lastTime;
	String firstName, lastName;

	for (int i = 0; i < procCount(); ++i) {
		const Time t = proc(i).startTime();
		if (t < 0)
			continue;

		if (firstTime < 0 || t < firstTime) {
			firstTime = t;
			firstName = proc(i).name();
		}

		if (lastTime < 0 || lastTime < t) {
			lastTime = t;
			lastName = proc(i).name();
		}
	}

	const Time diff = lastTime - firstTime;
	if (diff > Time::Sec(5*60)) {
		cerr << "warning: " << name() << "-side processes were started"
			" with a " << diff << " gap" << endl;
		cerr << "\tfirst process to start: " << firstName;
		HttpDatePrint(cerr << " at ", firstTime);
		cerr << "\tlast process to start: " << lastName;
		HttpDatePrint(cerr << " at ", lastTime);
	}

	theStartTime = firstTime; // regardless of the diff?
}

void SideInfo::checkCommonPhases() {
	if (procCount()) {
		const ProcInfo &p = proc(0);
		bool mismatch = false;
		for (int i = 1; i < procCount(); ++i) {
			if (p.phaseCount() != proc(i).phaseCount()) {
				mismatch = true;
				cerr << p.name() << " has " << p.phaseCount() << " phases"
					<< " while " << proc(i).name() << " has " 
					<< proc(i).phaseCount() << endl;
			}

			const int pCount = Min(p.phaseCount(), proc(i).phaseCount());
			for (int n = 0; n < pCount; ++n) {
				if (p.phase(n).name() != proc(i).phase(n).name()) {
					mismatch = true;
					cerr << "phase " << n << " in " << p.name() 
						<< " is named " << p.phase(n).name() << " while"
						<< " phase " << n << " in " << proc(i).name()
						<< " is named " << proc(i).phase(n).name() << endl;
				}
			}
		}

		if (mismatch) {
			cerr << "phase mismatch detected; any report information based"
				<< " on phase aggregation is likely to be wrong" << endl;
		}
	}
}

void SideInfo::checkConsistency() {
	for (int i = 0; i < procCount(); ++i)
		proc(i).checkConsistency();

	checkCommonBenchmarkVersion();
	checkCommonPglCfg();
	checkCommonStartTime();
	checkCommonPhases();
}

void SideInfo::CompileEmptyStats(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "side stats";
	ReportBlob blob(BlobDb::Key("summary", scope), tlTitle);
	XmlParagraph para;
	XmlText text;
	text.buf() << "no side information was extracted from the logs";
	para << text;
	blob << para;
	db << blob;
}

void SideInfo::compileStats(BlobDb &db) {
	clog << "compiling statistics for the " << name() << " side" << endl;

	for (int i = 0; i < theProcs.count(); ++i) {
		theProcs[i]->compileStats(db);
		theExecScopePhase.merge(theProcs[i]->execScopePhase());
		theAllPhasesPhase.merge(theProcs[i]->allPhasesPhase());
	}

	bool gotExecScope = false;
	for (int i = 0; i < phaseCount(); ++i) {
		PhaseInfo &phase = *thePhases[i];
		Scope phScope = scope().onePhase(phase.name());
		phScope.name(phase.name());
		gotExecScope = gotExecScope ||
			phScope.image() == execScope().image();
		compileStats(db, phase, phScope);
	}
	if (!gotExecScope)
		compileStats(db, theExecScopePhase, execScope());
	// else should copy existing phase(i) stats?

	compileStats(db, theAllPhasesPhase, theScope);

	cmplSideSum(db);
}

void SideInfo::compileStats(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	theUnseenObjects.clear();

	const String sfx = BlobDb::KeySuffix(scope);
	const StatIntvlRec &stats = phase.availStats();

	clog << "\t scope: " << '"' << scope.name() << '"' << endl;
	if (!phase.hasStats())
		clog << "\t\twarning: no phase statistics stored in this scope" << endl;

	addMeasBlob(db, "xact.count" + sfx, stats.xactCnt(), "xact", "transaction count");
	addMeasBlob(db, "xact.error.count" + sfx, stats.theXactErrCnt, "xact", "erroneous xaction count");
	addMeasBlob(db, "xact.error.ratio" + sfx, stats.errPercent(), "%", "portion of erroneous transactions");
	addMeasBlob(db, "duration" + sfx, stats.theDuration, "test duration");

	addMeasBlob(db, "offered.hit.ratio.obj" + sfx, stats.theIdealHR.dhp(), "%", "offered document hit ratio");
	addMeasBlob(db, "offered.hit.ratio.byte" + sfx, stats.theIdealHR.bhp(), "%", "offered byte hit ratio");
	addMeasBlob(db, "hit.ratio.obj" + sfx, stats.theRealHR.dhp(), "%", "measured document hit ratio");
	addMeasBlob(db, "hit.ratio.byte" + sfx, stats.theRealHR.bhp(), "%", "measured byte hit ratio");

	addMeasBlob(db, "req.rate" + sfx, stats.reqRate(), "xact/sec", "offered request rate");
	addMeasBlob(db, "rep.rate" + sfx, stats.repRate(), "xact/sec", "measured response rate");
	addMeasBlob(db, "req.bwidth" + sfx, stats.reqBwidth()/(1024*1024/8), "Mbits/sec", "request bandwidth");
	addMeasBlob(db, "rep.bwidth" + sfx, stats.repBwidth()/(1024*1024/8), "Mbits/sec", "response bandwidth");
	addMeasBlob(db, "rep.rptm.mean" + sfx, Time::Secd(stats.repTime().mean()/1000.), "mean response time");

	cmpProtoStats(db, phase, &StatIntvlRec::theSocksStat, scope);
	cmpProtoStats(db, phase, &StatIntvlRec::theSslStat, scope);
	cmpProtoStats(db, phase, &StatIntvlRec::theFtpStat, scope);

	addMeasBlob(db, "conn.count" + sfx,
		stats.theConnUseCnt.count(),
		"conn", "connection count"),
	addMeasBlob(db, "conn.pipeline.count" + sfx,
		stats.theConnPipelineDepth.count(), 
		"conn", "pipelined connection count");
	addMeasBlob(db, "conn.pipeline.ratio" + sfx, 
		Percent(stats.theConnPipelineDepth.count(), 
			stats.theConnUseCnt.count()), 
		"%", "portion of pipelined connections");

	addMeasBlob(db, "conn.pipeline.depth.min" + sfx,
		stats.theConnPipelineDepth.min(),
		"xact/pipe", "minimum transactions in pipeline");
	addMeasBlob(db, "conn.pipeline.depth.max" + sfx,
		stats.theConnPipelineDepth.max(),
		"xact/pipe", "maximum transactions in pipeline");
	addMeasBlob(db, "conn.pipeline.depth.mean" + sfx,
		stats.theConnPipelineDepth.mean(),
		"xact/pipe", "mean transactions in pipeline");

	addMeasBlob(db, "url.unique.count" + sfx, stats.theUniqUrlCnt, "xact", "unique URL count");
	addMeasBlob(db, "url.recurrence.ratio" + sfx, stats.recurrenceRatio(), "%", "recurrence ratio");

	cmplLoadBlob(db, scope);
	cmplProtoLoadBlob(db, phase, &StatIntvlRec::theSocksStat, scope);
	cmplProtoLoadBlob(db, phase, &StatIntvlRec::theSslStat, scope);
	cmplProtoLoadBlob(db, phase, &StatIntvlRec::theFtpStat, scope);
	cmplRptmFigure(db, scope);
	cmplRptmVsLoadFigure(db, phase, scope);
	cmplHitRatioTable(db, scope);
	cmplXactLevelTable(db, phase, scope);
	cmplConnLevelTable(db, phase, scope);
	cmplConnPipelineBlob(db, scope);
	cmplPopulLevelTable(db, phase, scope);
	cmplReplyStreamTable(db, phase, scope);
	cmplRequestStreamTable(db, phase, scope);
	cmplReplyObjectTable(db, phase, scope);
	cmplRequestObjectTable(db, phase, scope);
	cmplValidationTable(db, phase, scope);
	cmplErrorTable(db, phase, scope);
	cmplCompoundReplyStreamTable(db, phase, scope);
	cmplCompoundRequestStreamTable(db, phase, scope);
	cmplCompoundReplyObjectTable(db, phase, scope);
	cmplCompoundRequestObjectTable(db, phase, scope);
	cmplAuthStreamTable(db, phase, scope);
	cmplAuthObjectTable(db, phase, scope);
	cmplReplyStatusStreamTable(db, phase, scope);
	cmplReplyStatusObjectTable(db, phase, scope);
	cmplCookieTable(db, phase, scope);
	cmplObjectBlobs(db, phase, scope, TheReplyStex);
	cmplObjectBlobs(db, phase, scope, TheRequestStex);
	cmplObjectBlobs(db, phase, scope, TheCompoundReplyStex);
	cmplObjectBlobs(db, phase, scope, TheCompoundRequestStex);
	cmplObjectBlobs(db, phase, scope, TheAuthStex);
	cmplObjectBlobs(db, phase, scope, TheReplyStatusStex);
	cmplUnseenObjectsBlob(db, scope);
}

void SideInfo::cmplLoadBlob(BlobDb &db, const Scope &scope) {
	ReportBlob blob(BlobDb::Key("load", scope), ReportBlob::NilTitle);
	blob << XmlAttr("vprimitive", "Load");

	cmplLoadTable(db, blob, scope);
	cmplLoadFigure(db, blob, scope);

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The load table shows offered and measured load from "
			<< name() << " side point of view. Offered load statistics " 
			<< "are based on the request stream. Measured load statistics " 
			<< "are based on reply messages. The 'count' column depicts the "
			<< "number of requests or responses. ";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "The 'volume' column is a little bit more tricky to "
			<< "interpret. Offered volume is "
			<< "reply bandwidth that would have been required to support "
			<< "offered load. This volume is computed as request rate "
			<< "multiplied by measured mean response size. "
			<< "Measured volume is the actual or measured reply bandwidth.";
		descr << p2;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplLoadTable(BlobDb &db, ReportBlob &parent, const Scope &scope) {
	ReportBlob blob(BlobDb::Key("load.table", scope), name() + " load table");

	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Load");

		XmlTableHeading dhr("Count");
		dhr << XmlTag("br") << XmlText("(xact/sec)");
		tr << dhr;

		XmlTableHeading bhr("Volume");
		bhr << XmlTag("br") << XmlText("(Mbits/sec)");
		tr << bhr;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("offered");

		XmlTableCell cnt;
		cnt << algnRight << db.quote("req.rate" + scope);
		tr << cnt;

		XmlTableCell vol;
		vol << algnRight << db.quote("req.bwidth" + scope);
		tr << vol;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("measured");

		XmlTableCell cnt;
		cnt << algnRight << db.quote("rep.rate" + scope);
		tr << cnt;

		XmlTableCell vol;
		vol << algnRight << db.quote("rep.bwidth" + scope);
		tr << vol;

		table << tr;
	}

	blob << table;
	db << blob;
	parent << blob;
}

void SideInfo::cmplLoadFigure(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	SideLoadStex stex1("req", "offered", &StatIntvlRec::reqRate, &StatIntvlRec::reqBwidth);
	SideLoadStex stex2("rep", "measured", &StatIntvlRec::repRate, &StatIntvlRec::repBwidth);
	LoadTraceFig fig;
	fig.configure("load.trace" + scope, "load trace");
	fig.stats(&stex1, &phase(scope));
	fig.compareWith(&stex2);
	fig.globalStart(theTest->startTime());
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmpProtoStats(BlobDb &db, const PhaseInfo &phase, 
	ProtoIntvlPtr protoPtr, const Scope &scope) {

	const String sfx = BlobDb::KeySuffix(scope);
	const StatIntvlRec &stats = phase.availStats();
	const ProtoIntvlStat &protoStat = stats.*protoPtr;

	const double reqRate = protoStat.reqRate(stats.theDuration);
	const double repRate = protoStat.repRate(stats.theDuration);
	const double reqBwidth = protoStat.reqBwidth(stats.theDuration)/(1024*1024/8);
	const double repBwidth = protoStat.repBwidth(stats.theDuration)/(1024*1024/8);

	const String pfx = protoStat.id();
	const String protoName = protoStat.name();
	addMeasBlob(db, pfx + ".req.rate" + sfx, reqRate, "xact/sec", "offered " + protoName + " request rate");
	addMeasBlob(db, pfx + ".rep.rate" + sfx, repRate, "xact/sec", "measured " + protoName + " response rate");
	addMeasBlob(db, pfx + ".req.bwidth" + sfx, reqBwidth, "Mbits/sec", protoName + " request bandwidth");
	addMeasBlob(db, pfx + ".rep.bwidth" + sfx, repBwidth, "Mbits/sec", protoName + " response bandwidth");
}

void SideInfo::cmplProtoLoadBlob(BlobDb &db, const PhaseInfo &phase, ProtoIntvlPtr protoPtr, const Scope &scope) {
	const ProtoIntvlStat &protoStat = phase.availStats().*protoPtr;
	const String pfx = protoStat.id();
	const String protoName = protoStat.name();

	ReportBlob blob(BlobDb::Key(pfx + ".load", scope), ReportBlob::NilTitle);
	blob << XmlAttr("vprimitive", protoName + " load");

	cmplProtoLoadTable(db, blob, phase, protoPtr, scope);
	cmplProtoLoadFigure(db, blob, phase, protoPtr, scope);

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The " << protoName << " load table "
			<< "shows offered and measured load from "
			<< name() << " side point of view. Offered load statistics " 
			<< "are based on the request stream. Measured load statistics " 
			<< "are based on reply messages. The 'count' column depicts the "
			<< "number of requests or responses. ";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "The 'volume' column is a little bit more tricky to "
			<< "interpret. Offered volume is "
			<< "reply bandwidth that would have been required to support "
			<< "offered load. This volume is computed as request rate "
			<< "multiplied by measured mean response size. "
			<< "Measured volume is the actual or measured reply bandwidth.";
		descr << p2;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplProtoLoadTable(BlobDb &db, ReportBlob &parent, const PhaseInfo &phase, ProtoIntvlPtr protoPtr, const Scope &scope) {
	const ProtoIntvlStat &protoStat = phase.availStats().*protoPtr;
	const String pfx = protoStat.id();
	const String protoName = protoStat.name();

	ReportBlob blob(BlobDb::Key(pfx + ".load.table", scope), name() +
		protoName + " load table");

	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading(protoName + " load");

		XmlTableHeading dhr("Count");
		dhr << XmlTag("br") << XmlText("(xact/sec)");
		tr << dhr;

		XmlTableHeading bhr("Volume");
		bhr << XmlTag("br") << XmlText("(Mbits/sec)");
		tr << bhr;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("offered");

		XmlTableCell cnt;
		cnt << algnRight << db.quote(pfx + ".req.rate" + scope);
		tr << cnt;

		XmlTableCell vol;
		vol << algnRight << db.quote(pfx + ".req.bwidth" + scope);
		tr << vol;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("measured");

		XmlTableCell cnt;
		cnt << algnRight << db.quote(pfx + ".rep.rate" + scope);
		tr << cnt;

		XmlTableCell vol;
		vol << algnRight << db.quote(pfx + ".rep.bwidth" + scope);
		tr << vol;

		table << tr;
	}

	blob << table;
	db << blob;
	parent << blob;
}

void SideInfo::cmplProtoLoadFigure(BlobDb &db, ReportBlob &blob, const PhaseInfo &phase, ProtoIntvlPtr protoPtr, const Scope &scope) {
	const ProtoIntvlStat &protoStat = phase.availStats().*protoPtr;
	const String pfx = protoStat.id();
	const String protoName = protoStat.name();

	ProtoSideLoadStex stex1(pfx + ".req", protoName + " offered", protoPtr, &ProtoIntvlStat::reqRate, &ProtoIntvlStat::reqBwidth);
	ProtoSideLoadStex stex2(pfx + ".rep", protoName + " measured", protoPtr, &ProtoIntvlStat::repRate, &ProtoIntvlStat::repBwidth);
	LoadTraceFig fig;
	fig.configure(pfx + ".load.trace" + scope, protoName + " load trace");
	fig.stats(&stex1, &phase);
	fig.compareWith(&stex2);
	fig.globalStart(theTest->startTime());
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmplRptmFigure(BlobDb &db, const Scope &scope) {
	MissesStex misses("misses", "misses");
	HitsStex hits("hits", "hits");

	RptmTraceFig fig;
	fig.configure("rptm.trace" + scope, "Response times trace");
	fig.stats(&misses, &phase(scope));
	fig.moreStats(TheAllReps);
	fig.moreStats(&hits);
	fig.globalStart(theTest->startTime());
	//const String &figKey = 
	fig.plot(db).key();
	//blob << db.include(figKey);
}

void SideInfo::cmplRptmVsLoadFigure(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("rptm-load" + scope, "mean response time versus response rate");
	blob << XmlAttr("vprimitive", "Mean response time versus response rate");

	LoadPointStex load("rep", "response rate", "xact/sec", &StatIntvlRec::repRate);
	MeanRptmPointStex rptm;

	ScatteredFig fig;
	fig.configure("rptm-load.scatt" + scope, "Mean response time versus response rate");
	fig.stats(&load, &rptm, &phase);
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);

	db << blob;
}

void SideInfo::cmplHitRatioTable(BlobDb &db, const Scope &scope) {
	ReportBlob blob("hit.ratio" + scope, "hit ratios");
	blob << XmlAttr("vprimitive", "Client Side Hit Ratios");

	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Client Side Hit Ratios");

		XmlTableHeading dhr("DHR");
		dhr << XmlTag("br") << XmlText("(%)");
		tr << dhr;

		XmlTableHeading bhr("BHR");
		bhr << XmlTag("br") << XmlText("(%)");
		tr << bhr;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("offered");

		XmlTableCell dhr;
		dhr << algnRight << db.include("offered.hit.ratio.obj" + scope);
		tr << dhr;

		XmlTableCell bhr;
		bhr << algnRight << db.include("offered.hit.ratio.byte" + scope);
		tr << bhr;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("measured");

		XmlTableCell dhr;
		dhr << algnRight << db.include("hit.ratio.obj" + scope);
		tr << dhr;

		XmlTableCell bhr;
		bhr << algnRight << db.include("hit.ratio.byte" + scope);
		tr << bhr;

		table << tr;
	}

	blob << table;

	{
		XmlTag descr("description");

		if (name() == "client") {
			XmlTextTag<XmlParagraph> p1;
			p1.buf() << "The hit ratios table shows offered and measured hit "
				<< "ratios from " << name() << " side point of view. "
				<< "Polygraph counts every repeated request to a cachable "
				<< "object as an offered hit. Measured (cache) hits are "
				<< "detected using Polygraph-specific headers. All hits are "
				<< "counted for 'basic' transactions only (simple HTTP GET "
				<< "requests with '200 OK' responses).";
			descr << p1;

			XmlTextTag<XmlParagraph> p2;
			p2.buf() << "DHR, Document Hit Ratio, is the ratio of the total "
				<< "number of hits to the number of all basic transactions "
				<< "(hits and misses). BHR, Byte Hit Ratio, is the ratio of "
				<< "the total volume (a sum of response sizes) of hits to the "
				<< "total volume of all basic transactions.";
			descr << p2;
		} else {
			XmlTextTag<XmlParagraph> p1;
			p1.buf() << "The server-side hit ratios should always be zero. "
				<< "If a request reaches a server, it is, by definition, "
				<< "a miss.";
			descr << p1;
		}

		XmlParagraph p;
		Scope testScope = scope.oneSide("client");
		testScope.addSide("server");
		p << XmlText("A better way to measure hit ratio is to compare "
			"client- and server-side traffic. A hit ratio table "
			"based on such a comparison is available ");
		p << db.ptr("hit.ratio" + testScope, XmlText("elsewhere"));
		p << XmlText(".");
		descr << p;
		
		blob << descr;
	}

	cmplHrTraces(db, blob, scope);

	db << blob;
}

void SideInfo::cmplHrTraces(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	cmplDhrTrace(db, blob, scope);
	cmplBhrTrace(db, blob, scope);
}

void SideInfo::cmplDhrTrace(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	DhpPointStex stex1("dhp.ideal", "offered DHR", &StatIntvlRec::theIdealHR);
	DhpPointStex stex2("dhp.real", "measured DHR", &StatIntvlRec::theRealHR);

	PointTraceFig fig;
	fig.configure("dhr.trace" + scope, "Document hit ratio trace");
	fig.stats(&stex1, &stex2, &phase(scope));
	fig.globalStart(theTest->startTime());
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmplBhrTrace(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	BhpPointStex stex1("bhp.ideal", "offered BHR", &StatIntvlRec::theIdealHR);
	BhpPointStex stex2("bhp.real", "measured BHR", &StatIntvlRec::theRealHR);

	PointTraceFig fig;
	fig.configure("bhr.trace" + scope, "Byte hit ratio trace");
	fig.stats(&stex1, &stex2, &phase(scope));
	fig.globalStart(theTest->startTime());
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmplConnLevelTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("conn.level.table" + scope, "concurrent connection level");
	blob << XmlAttr("vprimitive", "Concurrent HTTP/TCP connection level table");

	const StatIntvlRec &stats = phase.availStats();

	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Connection state", 1, 2);
		tr << XmlTableHeading("Number of times", 2, 1);
		tr << XmlTableHeading("Mean concurrency level", 1, 2);
		table << tr;
	}

	{
		XmlTableRec tr;
		tr << XmlTableHeading("entered");
		tr << XmlTableHeading("left");
		table << tr;
	}

	cmplLevelTableRec(db, "conn.open.", "open", stats.theOpenLvl, scope, table);
	cmplLevelTableRec(db, "conn.estb.", "established", stats.theEstbLvl, scope, table);
	cmplLevelTableRec(db, "conn.idle.", "idle", stats.theIdleLvl, scope, table);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "TBD.";
		descr << p1;

		blob << descr;
	}

	// XXX: move
	cmplConnLevelFigure(db, blob, scope);

	db << blob;
}

void SideInfo::cmplConnLevelFigure(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	LevelStex stex1("open", "open", &StatIntvlRec::theOpenLvl);
	LevelStex stex2("estb", "established", &StatIntvlRec::theEstbLvl);
	LevelStex stex3("idle", "idle", &StatIntvlRec::theIdleLvl);
	LevelTraceFig fig;
	fig.configure("conn.level.trace" + scope, "concurrent HTTP/TCP connection level trace");
	fig.stats(&stex1, &phase(scope));
	fig.compareWith(&stex2);
	fig.compareWith(&stex3);
	fig.globalStart(theTest->startTime());
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmplConnPipelineBlob(BlobDb &db, const Scope &scope) {
	ReportBlob blob(BlobDb::Key("pipeline", scope), "Pipelined HTTP connections");
	blob << XmlAttr("vprimitive", "Pipelined HTTP connections");

	cmplConnPipelineTable(db, blob, scope);
	cmplConnPipelineHist(db, blob, scope);
	cmplConnPipelineTrace(db, blob, scope);

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "Connection pipelining stats are based on measurements " <<
			"collected for pipelined HTTP connections. To calculate " <<
			"pipelining probability, a connection is counted as pipelined " <<
			"if it had pipelined (concurrent) requests " <<
			"pending at any given moment of its lifetime.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "The pipeline " <<
			"depth varies as new requests are added to the connection and " <<
			"old requests are satisfied by responses. The depth reported " <<
			"her is based on the maximum pipelining depth achieved during " <<
			"a pipelined connection lifetime. That is, the depth stats are " <<
			"collected everytime a pipelined connection is closed, not " <<
			"when a new request is added to or removed from the pipe.";
		descr << p2;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplConnPipelineTable(BlobDb &db, ReportBlob &parent, const Scope &scope) {
	ReportBlob blob("conn.pipeline.table" + scope, "HTTP pipelining summary table");

	XmlTable table;
	table << XmlAttr::Int("border", 0) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("probability:");

		XmlTableCell cell;
		cell << db.include("conn.pipeline.ratio" + scope);
		cell << XmlText(" or ");
		cell << db.include("conn.pipeline.count" + scope);
		cell << XmlText(" pipelined out of total ");
		cell << db.include("conn.count" + scope);
		tr << cell;

		table << tr;
	}

	if (phase(scope).availStats().theConnPipelineDepth.known()) {
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("depth:");

		XmlTableCell cell;
		cell << db.include("conn.pipeline.depth.min" + scope);
		cell << XmlText(" min, ");
		cell << db.include("conn.pipeline.depth.mean" + scope);
		cell << XmlText(" mean, and ");
		cell << db.include("conn.pipeline.depth.max" + scope);
		cell << XmlText(" max");
		tr << cell;

		table << tr;
	}

	blob << table;

	db << blob;
	parent << blob;
}

void SideInfo::cmplConnPipelineTrace(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	PipelineProbPointStex stex1;
	MeanAggrPointStex stex2("depth", "pipeline depth", "connections", &StatIntvlRec::theConnPipelineDepth);

	PointTraceFig fig;
	fig.configure("conn.pipeline.trace" + scope, "HTTP pipelining trace");
	fig.stats(&stex1, &stex2, &phase(scope));
	fig.globalStart(theTest->startTime());
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmplConnPipelineHist(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	PipelineDepthHistStex stex1;

	HistogramFigure fig;
	fig.configure("conn.pipeline.depth.histogram" + scope, "HTTP pipelining depth histogram");
	fig.stats(&stex1, &phase(scope));
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmplPopulLevelTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("populus.level.table" + scope, "population level");
	blob << XmlAttr("vprimitive", "Population level table");

	const StatIntvlRec &stats = phase.availStats();

	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Number of agents", 2, 1);
		tr << XmlTableHeading("Mean population level", 1, 2);
		table << tr;
	}

	{
		XmlTableRec tr;
		tr << XmlTableHeading("created");
		tr << XmlTableHeading("destroyed");
		table << tr;
	}

	cmplLevelTableRec(db, "agent.", 0, stats.thePopulusLvl, scope, table);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "Populus is a set of all live robot or server agents. "
			<< "While alive, an agent may participate in HTTP transactions "
			<< "or remain idle.";
		descr << p1;

		blob << descr;
	}

	// XXX: move
	cmplPopulLevelFigure(db, blob, scope);

	db << blob;
}

void SideInfo::cmplPopulLevelFigure(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	LevelStex stex1("populus", "agents", &StatIntvlRec::thePopulusLvl);
	LevelTraceFig fig;
	fig.configure("populus.level.trace" + scope, "population level trace");
	fig.stats(&stex1, &phase(scope));
	fig.globalStart(theTest->startTime());
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmplXactLevelTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("xact.level.table" + scope, "concurrent transaction level");
	blob << XmlAttr("vprimitive", "Concurrent HTTP transaction level table");

	const StatIntvlRec &stats = phase.availStats();

	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Transaction state", 1, 2);
		tr << XmlTableHeading("Number of times", 2, 1);
		tr << XmlTableHeading("Mean concurrency level", 1, 2);
		table << tr;
	}

	{
		XmlTableRec tr;
		tr << XmlTableHeading("entered");
		tr << XmlTableHeading("left");
		table << tr;
	}

	cmplLevelTableRec(db, "xact.", "active", stats.theXactLvl, scope, table);
	cmplLevelTableRec(db, "wait.", "waiting", stats.theWaitLvl, scope, table);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "TBD.";
		descr << p1;

		blob << descr;
	}

	// XXX: move
	cmplXactLevelFigure(db, blob, scope);

	db << blob;
}

void SideInfo::cmplXactLevelFigure(BlobDb &db, ReportBlob &blob, const Scope &scope) {
	LevelStex stex1("xact", "active", &StatIntvlRec::theXactLvl);
	LevelStex stex2("wait", "waiting", &StatIntvlRec::theWaitLvl);
	LevelTraceFig fig;
	fig.configure("xact.level.trace" + scope, "concurrent HTTP transaction level trace");
	fig.stats(&stex1, &phase(scope));
	fig.compareWith(&stex2);
	fig.globalStart(theTest->startTime());
	const String &figKey = fig.plot(db).key();
	blob << db.include(figKey);
}

void SideInfo::cmplLevelTableRec(BlobDb &db, const String &pfx, const String &state, const LevelStat &stats, const Scope &scope, XmlTable &table) {
	XmlTableRec tr;
	if (state)
		tr << algnLeft << XmlTableHeading(state);

	const String startedName = pfx + "started" + scope;
	addMeasBlob(db, startedName, stats.incCnt(), "", "started");
	XmlTableCell started;
	started << algnRight << db.include(startedName);
	tr << started;

	const String finishedName = pfx + "finished" + scope;
	addMeasBlob(db, finishedName, stats.decCnt(), "", "finished");
	XmlTableCell finished;
	finished << algnRight << db.include(finishedName);
	tr << finished;

	const String levelName = pfx + "level.mean" + scope;
	addMeasBlob(db, levelName, stats.mean(), "", "average level");
	XmlTableCell level;
	level << algnRight << db.include(levelName);
	tr << level;

	table << tr;
}

XmlTable SideInfo::makeStreamTableHdr(const bool hasParts) {
	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Stream", 1, 2);
		tr << XmlTableHeading("Contribution", 2, 1);
		tr << XmlTableHeading("Rates", 2, 1);
		tr << XmlTableHeading("Totals", 2, 1);
		if (hasParts)
			tr << XmlTableHeading("Parts", 1, 1);
		table << tr;
	}

	{
		XmlTableRec tr;

		XmlTableHeading cCnt("Count");
		cCnt << XmlTag("br") << XmlText("(%)");
		tr << cCnt;

		XmlTableHeading cVol("Volume");
		cVol << XmlTag("br") << XmlText("(%)");
		tr << cVol;

		XmlTableHeading rCnt("Count");
		rCnt << XmlTag("br") << XmlText("(xact/sec)");
		tr << rCnt;

		XmlTableHeading rVol("Volume");
		rVol << XmlTag("br") << XmlText("(Mbits/sec)");
		tr << rVol;

		XmlTableHeading tCnt("Count");
		tCnt << XmlTag("br") << XmlText("(xact,M)");
		tr << tCnt;

		XmlTableHeading tVol("Volume");
		tVol << XmlTag("br") << XmlText("(Gbyte)");
		tr << tVol;

		if (hasParts) {
			XmlTableHeading pCnt("Count");
			pCnt << XmlTag("br") << XmlText("(xact/sec)");
			tr << pCnt;
		}

		table << tr;
	}

	return table;
}

void SideInfo::cmplReplyStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("reply_stream.table" + scope, "reply traffic stream");
	blob << XmlAttr("vprimitive", "Reply traffic stream table");

	StatTable table(makeStreamTableHdr());

	for (int s = 0; s < TheReplyStex.count(); ++s)
		cmplStreamTableRec(db, table, *TheReplyStex[s], phase, scope, TheAllReps);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'Reply stream' table provides count and volume "
			<< "statistics for many classes of transactions and for "
			<< "so-called pages.  The "
			<< "'Contribution' columns show count- and volume-based "
			<< "portions of all transactions. The 'Rates' columns show "
			<< "throughput and bandwidth measurements. The 'Totals' "
			<< "columns contain the total number of transactions "
			<< "and the total volume (a sum of individual response "
			<< "sizes) for each stream.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Note that some streams are a combination of other "
			<< "streams. For example, the 'all ims' stream contains "
			<< "transactions with If-Modified-Since requests that resulted in "
			<< "either '200 OK' (the 'ims/304' stream) or "
			<< "'304 Not Modified' (the 'ims/304' stream) responses. ";
		descr << p2;

		XmlTextTag<XmlParagraph> p3;
		p3.buf() << "Many combination streams, such as 'all content types' "
			<< "or 'hits and misses' stream, contribute less than 100% "
			<< "because properties like content type or hit status are "
			<< "distinguished for 'basic' transactions only. A basic "
			<< "transactions is a simple HTTP GET request resulted in "
			<< "a '200 OK' response. Various special transactions such "
			<< "as IMS or aborts do not belong to the 'basic' category.";
		descr << p3;

		XmlParagraph p4;
		p4 << XmlText("The ");
		p4 << db.ptr("reply_object.table" + scope, XmlText("'Reply object' table"));
		p4 << XmlText(" contains corresponding response time and size "
			"statistics for streams.");
		descr << p4;

		XmlParagraph p5;
		p5 << XmlText("A similar table covering request messages is available ");
		p5 << db.ptr("request_stream.table" + scope, XmlText("elsewhere"));
		p5 << XmlText(".");
		descr << p5;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplRequestStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("request_stream.table" + scope, "request traffic stream");
	blob << XmlAttr("vprimitive", "Request traffic stream table");

	StatTable table(makeStreamTableHdr());

	Array<Stex*> a;
	SortStexes(phase, TheRequestStex, a);
	for (int s = 0; s < a.count(); ++s)
		cmplStreamTableRec(db, table, *a[s], phase, scope, TheAllReqs);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'Request stream' table provides count and volume "
			<< "statistics for requests.  The "
			<< "'Contribution' columns show count- and volume-based "
			<< "portions of all transactions. The 'Rates' columns show "
			<< "throughput and bandwidth measurements. The 'Totals' "
			<< "columns contain the total number of transactions "
			<< "and the total volume (a sum of individual request "
			<< "sizes) for each stream.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Note that some streams are a combination of other "
			<< "streams. For example, the 'all request content types' stream contains "
			<< "requests with different content types.";
		descr << p2;

		XmlTextTag<XmlParagraph> p3;
		p3.buf() << "Note that only request messages containing "
			"bodies contribute to these stats at the moment.";
		descr << p3;

		XmlParagraph p4;
		p4 << XmlText("The ");
		p4 << db.ptr("request_object.table" + scope, XmlText("'Request object' table"));
		p4 << XmlText(" contains corresponding response time and size "
			"statistics for streams.");
		descr << p4;

		XmlParagraph p5;
		p5 << XmlText("A similar table covering reply messages is available ");
		p5 << db.ptr("reply_stream.table" + scope, XmlText("elsewhere"));
		p5 << XmlText(".");
		descr << p5;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplCompoundReplyStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("compound.reply_stream.table" + scope, "compound reply traffic stream");
	blob << XmlAttr("vprimitive", "Compound reply traffic stream table");

	StatTable table(makeStreamTableHdr(true));

	for (int s = 0; s < TheCompoundReplyStex.count(); ++s)
		cmplStreamTableRec(db, table, *TheCompoundReplyStex[s], phase, scope, TheAllReps);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "A compound transaction consists of related "
			"transactions, working on a single goal such as "
			"authenticating a transfer. While individual "
			"transactions consist of a single request/response "
			"pair, compound transactions usually have several "
			"such pairs. Isolated transactions are individual "
			"transactions that do not belong to any compound "
			"transaction.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Stats in the 'Compound reply stream' table "
			"provides count and volume statistics for responses. "
			"The 'Contribution' columns show count- and volume-based "
			"portions of all compound transactions (isolated "
			"transaction not included). The 'Rates' columns show "
			"throughput and bandwidth measurements. The 'Totals' "
			"columns contain the total number of transactions and the "
			"total volume (a sum of individual request sizes) for "
			"each stream. The 'Parts' column shows individual "
			"transactions rate in a compound transaction.";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("The ");
		p3 << db.ptr("compound.reply_object.table" + scope, XmlText("'Compound reply object' table"));
		p3 << XmlText(" contains corresponding response time and size "
			"statistics for streams.");
		descr << p3;

		XmlParagraph p4;
		p4 << XmlText("A similar table covering request messages is available ");
		p4 << db.ptr("compound.request_stream.table" + scope, XmlText("elsewhere"));
		p4 << XmlText(".");
		descr << p4;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplCompoundRequestStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("compound.request_stream.table" + scope, "compound request traffic stream");
	blob << XmlAttr("vprimitive", "Compound request traffic stream table");

	StatTable table(makeStreamTableHdr(true));

	for (int s = 0; s < TheCompoundRequestStex.count(); ++s)
		cmplStreamTableRec(db, table, *TheCompoundRequestStex[s], phase, scope, TheAllReqs);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "A compound transaction consists of related "
			"transactions, working on a single goal such as "
			"authenticating a transfer. While individual "
			"transactions consist of a single request/response "
			"pair, compound transactions usually have several "
			"such pairs. Isolated transactions are individual "
			"transactions that do not belong to any compound "
			"transaction.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Stats in the 'Compound request stream' table "
			"provides count and volume statistics for requests. "
			"The 'Contribution' columns show count- and volume-based "
			"portions of all compound transactions (isolated "
			"transaction not included). The 'Rates' columns show "
			"throughput and bandwidth measurements. The 'Totals' "
			"columns contain the total number of transactions and the "
			"total volume (a sum of individual request sizes) for "
			"each stream. The 'Parts' column shows individual "
			"transactions rate in a compound transaction.";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("The ");
		p3 << db.ptr("compound.request_object.table" + scope, XmlText("'Compound request object' table"));
		p3 << XmlText(" contains corresponding request time and size "
			"statistics for streams.");
		descr << p3;

		XmlParagraph p4;
		p4 << XmlText("A similar table covering reply messages is available ");
		p4 << db.ptr("compound.reply_stream.table" + scope, XmlText("elsewhere"));
		p4 << XmlText(".");
		descr << p4;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplAuthStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("auth.stream.table" + scope, "authentication traffic stream");
	blob << XmlAttr("vprimitive", "Authentication traffic stream table");

	StatTable table(makeStreamTableHdr());

	for (int s = 0; s < TheAuthStex.count(); ++s)
		cmplStreamTableRec(db, table, *TheAuthStex[s], phase, scope, TheAllReps);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'Authentication stream' table provides count "
			"and volume statistics for authentication-related "
			"transactions. The 'Contribution' columns show count- "
			"and volume-based portions of all transactions. The "
			"'Rates' columns show throughput and bandwidth "
			"measurements. The 'Totals' columns contain the total "
			"number of transactions and the total volume (a sum of "
			"individual response sizes) for each stream.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Note that some streams are a combination of other "
			"streams. For example, the 'all auth-ing' stream contains "
			"auth-ing transactions with any authentication scheme.";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("The ");
		p3 << db.ptr("auth.object.table" + scope, XmlText("'Authentication object' table"));
		p3 << XmlText(" contains corresponding response time and size "
			"statistics for streams.");
		descr << p3;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplReplyStatusStreamTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob("reply_status.stream.table" + scope, "reply status traffic stream");
	blob << XmlAttr("vprimitive", "HTTP reply status traffic stream table");

	StatTable table(makeStreamTableHdr());

	Array<Stex*> a;
	SortStexes(phase, TheReplyStatusStex, a);
	for (int s = 0; s < a.count(); ++s)
		cmplStreamTableRec(db, table, *a[s], phase, scope, TheReplyStatusStex.last());

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'HTTP reply status stream' table provides "
			"count and volume statistics for HTTP responses with "
			"different status codes. The 'Contribution' columns "
			"show count- and volume-based portions of all replies. "
			"The 'Rates' columns show throughput and bandwidth "
			"measurements. The 'Totals' columns contain the total "
			"number of transactions and the total volume (a sum of "
			"individual response sizes) for each stream. 'Other "
			"HTTP reply status code' stream contains transactions "
			"with errors, e.g. parsing or connection failure, "
			"where response status code is unknown.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "The 'All HTTP reply status codes' stream contains "
			"statistics for responses with all status codes.";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("The ");
		p3 << db.ptr("reply_status.object.table" + scope, XmlText("'HTTP reply status codes' table"));
		p3 << XmlText(" contains corresponding response time and size statistics.");
		descr << p3;

		blob << descr;
	}

	db << blob;
}

// addMeasBlob() calls should be moved out if we want to support partial reports
void SideInfo::cmplStreamTableRec(BlobDb &db, StatTable &table, const Stex &stex, const PhaseInfo &phase, const Scope &scope, const Stex *const topStex) {
	const String pfx = "stream." + stex.key();
	const String ratioCountName = BlobDb::Key(pfx + ".ratio.obj", scope);
	const String ratioVolumeName = BlobDb::Key(pfx + ".ratio.byte", scope);
	const String rateCountName = BlobDb::Key(pfx + ".rate", scope);
	const String rateVolumeName = BlobDb::Key(pfx + ".bwidth", scope);
	const String totalCountName = BlobDb::Key(pfx + ".size.count", scope);
	const String totalVolumeName = BlobDb::Key(pfx + ".size.sum", scope);
	const String partsRateCountName = BlobDb::Key(pfx + ".parts.rate", scope);

	const String ratioCountTitle = "contribution by count";
	const String ratioVolumeTitle = "contribution by volume";
	const String rateCountTitle = "transaction rate";
	const String rateVolumeTitle = "transaction bandwidth";
	const String totalCountTitle = "total transaction count";
	const String totalVolumeTitle = "total transaction volume";

	const Time duration = phase.availStats().theDuration;
	const AggrStat *const partsStat = stex.partsStat(phase);
	bool known(false);
	if (const TmSzStat *const recStats = stex.aggr(phase)) {
		const AggrStat &cstats = recStats->size();
		known = cstats.known();

		const double rateCountVal = Ratio(cstats.count(), duration.secd());
		const double rateVolumeVal = Ratio(cstats.sum()/1024/1024*8, duration.secd());
		const double totalCountVal = cstats.count();
		const double totalVolumeVal = cstats.sum();

		if ((stex.parent() || &stex == topStex) &&
			topStex->aggr(phase)) {
			// compute contribution towards "all responses"
			const AggrStat all = topStex->aggr(phase)->size();
			const double partsCount = stex.meanPartsCount(phase);
			addMeasBlob(db, ratioCountName, Percent(totalCountVal * partsCount, all.count()), "%", ratioCountTitle);
			addMeasBlob(db, ratioVolumeName, Percent(totalVolumeVal, all.sum()), "%", ratioVolumeTitle);
		} else {
			addNaMeasBlob(db, ratioCountName, ratioCountTitle);
			addNaMeasBlob(db, ratioVolumeName, ratioVolumeTitle);
		}

		addMeasBlob(db, rateCountName, rateCountVal, "/sec", rateCountTitle);
		addMeasBlob(db, rateVolumeName, rateVolumeVal, "Mbits/sec", rateVolumeTitle);
		addMeasBlob(db, totalCountName, totalCountVal/1e6, "M", totalCountTitle);
		addMeasBlob(db, totalVolumeName, totalVolumeVal/(1024*1024*1024), "GByte", totalVolumeTitle);
	} else {
		addNaMeasBlob(db, ratioCountName, ratioCountTitle);
		addNaMeasBlob(db, ratioVolumeName, ratioVolumeTitle);
		addNaMeasBlob(db, rateCountName, rateCountTitle);
		addNaMeasBlob(db, rateVolumeName, rateVolumeTitle);
		addNaMeasBlob(db, totalCountName, totalCountTitle);
		addNaMeasBlob(db, totalVolumeName, totalVolumeTitle);
	}

	if (partsStat) {
		known = known || partsStat->known();
		const String partsRateCountTitle = "parts rate";
		const double partsRateCountVal = Ratio(partsStat->sum(), duration.secd());
		addMeasBlob(db, partsRateCountName, partsRateCountVal, "/sec", partsRateCountTitle);
	}

	if (!known) {
		table.addUnknown(stex);
		return;
	}

	XmlTableRec tr;

	XmlTableHeading th;
	th << db.ptr("object." + stex.key() + scope, XmlText(stex.name()));
	th << algnLeft;
	tr << th;

	XmlTableCell ratioCountCell;
	ratioCountCell << algnRight << db.quote(ratioCountName);
	tr << ratioCountCell;

	XmlTableCell ratioVolumeCell;
	ratioVolumeCell << algnRight << db.quote(ratioVolumeName);
	tr << ratioVolumeCell;

	XmlTableCell rateCountCell;
	rateCountCell << algnRight << db.quote(rateCountName);
	tr << rateCountCell;

	XmlTableCell rateVolumeCell;
	rateVolumeCell << algnRight << db.quote(rateVolumeName);
	tr << rateVolumeCell;

	XmlTableCell totalCountCell;
	totalCountCell << algnRight << db.quote(totalCountName);
	tr << totalCountCell;

	XmlTableCell totalVolumeCell;
	totalVolumeCell << algnRight << db.quote(totalVolumeName);
	tr << totalVolumeCell;

	if (partsStat) {
		XmlTableCell partsRateCountCell;
		partsRateCountCell << algnRight << db.quote(partsRateCountName);
		tr << partsRateCountCell;
	}

	table << tr;
}

XmlTable SideInfo::makeObjectTableHdr(const bool hasParts) {
	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Object", 1, 2);
		tr << XmlTableHeading("Response time (msec)", 3, 1);
		tr << XmlTableHeading("Size (KBytes)", 3, 1);
		if (hasParts)
			tr << XmlTableHeading("Parts", 3, 1);
		table << tr;
	}

	{
		XmlTableRec tr;

		XmlNodes nodes;

		nodes << XmlTableHeading("Min");
		nodes << XmlTableHeading("Mean");
		nodes << XmlTableHeading("Max");

		tr << nodes;
		tr << nodes;
		if (hasParts)
			tr << nodes;

		table << tr;
	}

	return table;
}

void SideInfo::cmplReplyObjectTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
{
	ReportBlob blob(BlobDb::Key("reply_object.table", scope), "response kind stats");
	blob << XmlAttr("vprimitive", "Reply object kind table");

	StatTable table(makeObjectTableHdr());

	for (int s = 0; s < TheReplyStex.count(); ++s)
		cmplObjectTableRec(db, table, *TheReplyStex[s], phase, scope);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'Reply object' table provides response time and response "
			<< "size statistics for many classes of transactions and "
			<< "for so-called pages.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Note that some classes are a combination of other "
			<< "classes. For example, the 'all ims' class contains "
			<< "transactions with If-Modified-Since requests that resulted in "
			<< "either '200 OK' (the 'ims/304' class) or "
			<< "'304 Not Modified' (the 'ims/304' class) responses. ";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("Some statistics may not be available because either "
			"no objects of the corresponding class were seen during the "
			"test or no facilities to collect the stats exist for "
			"the class. The former can be verified using a ");
		p3 << db.ptr("reply_stream.table" + scope, XmlText("'Reply stream' table"));
		p3 << XmlText(".");
		descr << p3;

		XmlParagraph p4;
		p4 << XmlText("A similar table covering request messages is available ");
		p4 << db.ptr("request_object.table" + scope, XmlText("elsewhere"));
		p4 << XmlText(".");
		descr << p4;

		blob << descr;
	}

	db << blob;
}

}

void SideInfo::cmplRequestObjectTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
{
	ReportBlob blob(BlobDb::Key("request_object.table", scope), "request kind stats");
	blob << XmlAttr("vprimitive", "Request object kind table");

	StatTable table(makeObjectTableHdr());

	for (int s = 0; s < TheRequestStex.count(); ++s)
		cmplObjectTableRec(db, table, *TheRequestStex[s], phase, scope);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'Request object' table provides time and "
			<< "size statistics for requests.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Note that some classes are a combination of other "
			<< "classes. For example, the 'all request content types' class contains "
			<< "requests with different content type.";
		descr << p2;

		XmlTextTag<XmlParagraph> p3;
		p3.buf() << "Note that only request messages containing "
			"bodies contribute to these stats at the moment.";
		descr << p3;

		XmlParagraph p4;
		p4 << XmlText("Some statistics may not be available because either "
			"no objects of the corresponding class were seen during the "
			"test or no facilities to collect the stats exist for "
			"the class. The former can be verified using a ");
		p4 << db.ptr("request_stream.table" + scope, XmlText("'Request stream' table"));
		p4 << XmlText(".");
		descr << p4;

		XmlParagraph p5;
		p5 << XmlText("A similar table covering reply messages is available ");
		p5 << db.ptr("reply_object.table" + scope, XmlText("elsewhere"));
		p5 << XmlText(".");
		descr << p5;

		blob << descr;
	}

	db << blob;
}

}

void SideInfo::cmplCompoundReplyObjectTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob(BlobDb::Key("compound.reply_object.table", scope), "compound response kind stats");
	blob << XmlAttr("vprimitive", "Compound reply object kind table");

	StatTable table(makeObjectTableHdr(true));

	for (int s = 0; s < TheCompoundReplyStex.count(); ++s)
		cmplObjectTableRec(db, table, *TheCompoundReplyStex[s], phase, scope);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "A compound transaction consists of related "
			"transactions, working on a single goal such as "
			"authenticating a transfer. While individual "
			"transactions consist of a single request/response "
			"pair, compound transactions usually have several "
			"such pairs. Isolated transactions are individual "
			"transactions that do not belong to any compound "
			"transaction.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Stats in the 'Compound reply object' table "
			"provides time and size statistics for responses. "
			"Compound transaction response time is time from "
			"the start of the first transaction until the end of "
			"the last one. Request (response) size is the total "
			"size of all individual requests (responses) in a "
			"compound transaction. The 'Parts' column shows "
			"the number of individual transactions in a compound "
			"transaction.";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("The ");
		p3 << db.ptr("compound.reply_stream.table" + scope, XmlText("'Compound reply stream' table"));
		p3 << XmlText(" contains corresponding stream statistics.");
		descr << p3;

		XmlParagraph p4;
		p4 << XmlText("A similar table covering request messages is available ");
		p4 << db.ptr("compound.request_object.table" + scope, XmlText("elsewhere"));
		p4 << XmlText(".");
		descr << p4;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplCompoundRequestObjectTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob(BlobDb::Key("compound.request_object.table", scope), "compound request kind stats");
	blob << XmlAttr("vprimitive", "Compound request object kind table");

	StatTable table(makeObjectTableHdr(true));

	for (int s = 0; s < TheCompoundRequestStex.count(); ++s)
		cmplObjectTableRec(db, table, *TheCompoundRequestStex[s], phase, scope);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "A compound transaction consists of related "
			"transactions, working on a single goal such as "
			"authenticating a transfer. While individual "
			"transactions consist of a single request/response "
			"pair, compound transactions usually have several "
			"such pairs. Isolated transactions are individual "
			"transactions that do not belong to any compound "
			"transaction.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Stats in the 'Compound request object' table "
			"provides time and size statistics for requests. "
			"Compound transaction response time is time from "
			"the start of the first transaction until the end of "
			"the last one. Request (response) size is the total "
			"size of all individual requests (responses) in a "
			"compound transaction. The 'Parts' column shows "
			"the number of individual transactions in a compound "
			"transaction.";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("The ");
		p3 << db.ptr("compound.request_stream.table" + scope, XmlText("'Compound request stream' table"));
		p3 << XmlText(" contains corresponding stream statistics.");
		descr << p3;

		XmlParagraph p4;
		p4 << XmlText("A similar table covering response messages is available ");
		p4 << db.ptr("compound.reply_object.table" + scope, XmlText("elsewhere"));
		p4 << XmlText(".");
		descr << p4;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplAuthObjectTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob(BlobDb::Key("auth.object.table", scope), "authentication kind stats");
	blob << XmlAttr("vprimitive", "Authentication object kind table");

	StatTable table(makeObjectTableHdr());

	for (int s = 0; s < TheAuthStex.count(); ++s)
		cmplObjectTableRec(db, table, *TheAuthStex[s], phase, scope);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'Authentication object' table provides response "
			"time and response size statistics authentication-related "
			"tansactions.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "Note that some streams are a combination of other "
			"streams. For example, the 'all auth-ing' stream contains "
			"auth-ing transactions with any authentication scheme.";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("Some statistics may not be available because either "
			"no objects of the corresponding class were seen during the "
			"test or no facilities to collect the stats exist for "
			"the class. The former can be verified using a ");
		p3 << db.ptr("auth.stream.table" + scope, XmlText("'Authentication stream' table"));
		p3 << XmlText(".");
		descr << p3;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplReplyStatusObjectTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob(BlobDb::Key("reply_status.object.table", scope), "reply status code stats");
	blob << XmlAttr("vprimitive", "HTTP reply status codes table");

	StatTable table(makeObjectTableHdr());

	for (int s = 0; s < TheReplyStatusStex.count(); ++s)
		cmplObjectTableRec(db, table, *TheReplyStatusStex[s], phase, scope);

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'HTTP reply status codes' table provides "
			"response time and response size statistics for "
			"HTTP transactions with different status codes. "
			"'Other HTTP reply status code' stream contains "
			"transactions with errors, e.g. parsing or connection "
			"failure, where response status code is unknown.";
		descr << p1;

		XmlTextTag<XmlParagraph> p2;
		p2.buf() << "The 'All HTTP reply status codes' stream contains "
			"statistics for responses with all status codes.";
		descr << p2;

		XmlParagraph p3;
		p3 << XmlText("See also: ");
		p3 << db.ptr("reply_status.stream.table" + scope, XmlText("'HTTP reply status stream' table"));
		p3 << XmlText(".");
		descr << p3;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplObjectTableRec(BlobDb &db, StatTable &table, const Stex &stex, const PhaseInfo &phase, const Scope &scope) {
	const String pfx = "object." + stex.key();
	const String rptmMinName = BlobDb::Key(pfx + ".rptm.min", scope);
	const String rptmMeanName = BlobDb::Key(pfx + ".rptm.mean", scope);
	const String rptmMaxName = BlobDb::Key(pfx + ".rptm.max", scope);
	const String sizeMinName = BlobDb::Key(pfx + ".size.min", scope);
	const String sizeMeanName = BlobDb::Key(pfx + ".size.mean", scope);
	const String sizeMaxName = BlobDb::Key(pfx + ".size.max", scope);
	const String partsMinName = BlobDb::Key(pfx + ".parts.min", scope);
	const String partsMeanName = BlobDb::Key(pfx + ".parts.mean", scope);
	const String partsMaxName = BlobDb::Key(pfx + ".parts.max", scope);

	const TmSzStat *cstats = stex.aggr(phase);
	const AggrStat *const partsStat = stex.partsStat(phase);
	bool known(false);

	{
		const String rptmMinTitle = "minimum response time";
		const String rptmMeanTitle = "mean response time";
		const String rptmMaxTitle = "maximum response time";

		if (cstats && cstats->time().known()) {
			known = true;
			const AggrStat &time = cstats->time();
			addMeasBlob(db, rptmMinName, time.min(), "msec", rptmMinTitle);
			addMeasBlob(db, rptmMeanName, time.mean(), "msec", rptmMeanTitle);
			addMeasBlob(db, rptmMaxName, time.max(), "msec", rptmMaxTitle);
		} else {
			addNaMeasBlob(db, rptmMinName, rptmMinTitle);
			addNaMeasBlob(db, rptmMeanName, rptmMeanTitle);
			addNaMeasBlob(db, rptmMaxName, rptmMaxTitle);
		}
	}

	{
		const String sizeMinTitle = "minimum size";
		const String sizeMeanTitle = "mean size";
		const String sizeMaxTitle = "maximum size";

		if (cstats && cstats->size().known()) {
			known = true;
			const AggrStat &size = cstats->size();
			addMeasBlob(db, sizeMinName, size.min()/1024., "KBytes", sizeMinTitle);
			addMeasBlob(db, sizeMeanName, size.mean()/1024., "KBytes", sizeMeanTitle);
			addMeasBlob(db, sizeMaxName, size.max()/1024., "KBytes", sizeMaxTitle);
		} else {
			addNaMeasBlob(db, sizeMinName, sizeMinTitle);
			addNaMeasBlob(db, sizeMeanName, sizeMeanTitle);
			addNaMeasBlob(db, sizeMaxName, sizeMaxTitle);
		}

		if (partsStat) {
			const String partsMinTitle = "minimum count";
			const String partsMeanTitle = "mean count";
			const String partsMaxTitle = "maximum count";

			if (partsStat->known()) {
				known = true;
				addMeasBlob(db, partsMinName, partsStat->min(), "xacts", partsMinTitle);
				addMeasBlob(db, partsMeanName, partsStat->mean(), "xacts", partsMeanTitle);
				addMeasBlob(db, partsMaxName, partsStat->max(), "xacts", partsMaxTitle);
			} else {
				addNaMeasBlob(db, partsMinName, partsMinTitle);
				addNaMeasBlob(db, partsMeanName, partsMeanTitle);
				addNaMeasBlob(db, partsMaxName, partsMaxTitle);
			}
		}
	}

	if (!known) {
		table.addUnknown(stex);
		return;
	}

	XmlTableRec tr;

	XmlTableHeading th;
	th << db.ptr(pfx + scope, XmlText(stex.name()));
	th << algnLeft;
	tr << th;

	XmlTableCell rptmMinCell;
	rptmMinCell << algnRight << db.quote(rptmMinName);
	tr << rptmMinCell;

	XmlTableCell rptmMeanCell;
	rptmMeanCell << algnRight << db.quote(rptmMeanName);
	tr << rptmMeanCell;

	XmlTableCell rptmMaxCell;
	rptmMaxCell << algnRight << db.quote(rptmMaxName);
	tr << rptmMaxCell;

	XmlTableCell sizeMinCell;
	sizeMinCell << algnRight << db.quote(sizeMinName);
	tr << sizeMinCell;

	XmlTableCell sizeMeanCell;
	sizeMeanCell << algnRight << db.quote(sizeMeanName);
	tr << sizeMeanCell;

	XmlTableCell sizeMaxCell;
	sizeMaxCell << algnRight << db.quote(sizeMaxName);
	tr << sizeMaxCell;

	if (partsStat) {
		XmlTableCell partsMinCell;
		partsMinCell << algnRight << db.quote(partsMinName);
		tr << partsMinCell;

		XmlTableCell partsMeanCell;
		partsMeanCell << algnRight << db.quote(partsMeanName);
		tr << partsMeanCell;

		XmlTableCell partsMaxCell;
		partsMaxCell << algnRight << db.quote(partsMaxName);
		tr << partsMaxCell;
	}

	table << tr;
}

void SideInfo::cmplValidationTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	const String pfx = BlobDb::Key("validation.table", scope);
	ReportBlob blob(pfx, "validation effectiveness");
	blob << XmlAttr("vprimitive", "Validation effectiveness table");

	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Validation method");

		XmlTableHeading cnt("Useful count");
		cnt << XmlTag("br") << XmlText("(%)");
		tr << cnt;

		XmlTableHeading vol("Useful volume");
		vol << XmlTag("br") << XmlText("(%)");
		tr << vol;

		table << tr;
	}

	cmplValidationTableRec(db, table, *TheUsefulProxyValidation, phase, scope, pfx + ".proxy_validation", "Proxy validation");

	blob << table;

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'Validation effectiveness' table shows useful "
			<< "request ratios for various validation methods. "
			<< "Polygraph counts every validation request that "
			<< "results in bodyless response as useless. Validation "
			<< "requests that result in response with body are "
                         << "useful.";
		descr << p1;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplValidationTableRec(BlobDb &db, XmlTable &table, const Stex &stex, const PhaseInfo &phase, const Scope &scope, const String &pfx, const String &name) {
	Assert(stex.parent());

	const String ratioCountName = BlobDb::Key(pfx + ".ratio.obj", scope);
	const String ratioVolumeName = BlobDb::Key(pfx + ".ratio.byte", scope);
	const String ratioCountTitle = "contribution by count";
	const String ratioVolumeTitle = "contribution by volume";

	if (stex.aggr(phase) &&
		stex.parent()->aggr(phase)) {
		const AggrStat &cstats = stex.aggr(phase)->size();
		const double totalCountVal = cstats.count();
		const double totalVolumeVal = cstats.sum();
		const AggrStat all = stex.parent()->aggr(phase)->size();

		addMeasBlob(db, ratioCountName, Percent(totalCountVal, all.count()), "%", ratioCountTitle);
		addMeasBlob(db, ratioVolumeName, Percent(totalVolumeVal, all.sum()), "%", ratioVolumeTitle);
	} else {
		addNaMeasBlob(db, ratioCountName, ratioCountTitle);
		addNaMeasBlob(db, ratioVolumeName, ratioVolumeTitle);
	}

	XmlTableRec tr;
	tr << algnLeft << XmlTableHeading(name);

	XmlTableCell ratioCountCell;
	ratioCountCell << algnRight << db.quote(ratioCountName);
	tr << ratioCountCell;

	XmlTableCell ratioVolumeCell;
	ratioVolumeCell << algnRight << db.quote(ratioVolumeName);
	tr << ratioVolumeCell;

	table << tr;
}

void SideInfo::cmplErrorTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	ReportBlob blob(BlobDb::Key("errors.table", scope), "error stats");
	blob << XmlAttr("vprimitive", "Errors");

	ErrorStat::Index idx;
	if (phase.hasStats() && phase.stats().theErrors.index(idx)) {
		XmlParagraph p;
		XmlText text;
		text.buf() << "The total of " << phase.stats().theErrors.count() 
			<< " errors detected. Out of those errors, ";
		p << text << db.include("xact.error.count" + scope);
		p << XmlText(" or ") << db.include("xact.error.ratio" + scope);
		p << XmlText(" of all transactions were classified as transaction errors.");
		blob << p;

		XmlTable table;
		table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

		{
			XmlTableRec tr;
			tr << XmlTableHeading("Error");
			tr << XmlTableHeading("Count");
			tr << XmlTableHeading("Contribution (%)");
			table << tr;
		}

		for (int i = 0; i < idx.count(); ++i)
			cmplErrorTableRec(db, table, phase.stats().theErrors, *idx[i], scope);

		blob << table;

		{
			XmlTag descr("description");

			XmlTextTag<XmlParagraph> p1;
			p1.buf() << "The 'Errors' table shows detected errors. For each " <<
				"error type, the number of errors and their contribution towards " <<
				"total error count are shown.";
			descr << p1;

			blob << descr;
		}
	} else
	if (phase.hasStats()) {
		blob << XmlTextTag<XmlParagraph>("No errors detected in the given scope.");
	} else {
		XmlParagraph p;
		p << XmlText("The total of ") << db.include("xact.error.count" + scope)
			<< XmlText(" or ") << db.include("xact.error.ratio" + scope);
		p << XmlText(" transaction errors detected.");
		blob << p;
	}

	db << blob;
}

void SideInfo::cmplErrorTableRec(BlobDb &, XmlTable &table, const ErrorStat &errors, const ErrorRec &error, const Scope &) {
	XmlTableRec tr;

	XmlTableHeading th;
	XmlText tht;
	error.print(tht.buf());
	th << algnLeft;
	th << tht;
	tr << th;

	XmlTableCell countCell;
	XmlText countText;
	countText.buf() << error.count();
	countCell << algnRight << countText;
	tr << countCell;

	XmlTableCell contribCell;
	XmlText contribText;
	contribText.buf() << Percent(error.count(), errors.count());
	contribCell << algnRight << contribText;
	tr << contribCell;

	table << tr;
}

void SideInfo::cmplCookieTable(BlobDb &db, const PhaseInfo &phase, const Scope &scope) {
	const String pfx = BlobDb::Key("cookie.table", scope);
	ReportBlob blob(pfx, "cookie stats");
	blob << XmlAttr("vprimitive", "HTTP Cookies table");

	{
		XmlTable table;
		table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

		{
			XmlTableRec tr;
			tr << XmlTableHeading("Stream", 1, 2);

			XmlTableHeading cnt("#Messages with cookies", 2, 1);
			tr << cnt;

			XmlTableHeading cookies("#Cookies in messages", 3, 1);
			tr << cookies;

			XmlTableHeading total("Total #cookies", 1, 2);
			tr << total;

			table << tr;
		}

		{
			XmlTableRec tr;

			XmlTableHeading absolute("Absolute");
			tr << absolute;

			XmlTableHeading relative("Relative");
			tr << relative;

			XmlTableHeading min("Min");
			tr << min;

			XmlTableHeading mean("Mean");
			tr << mean;

			XmlTableHeading max("Max");
			tr << max;

			table << tr;
		}

		{
			CookiesStex stex("cookie.sent", "Sent cookies", &StatPhaseRec::theCookiesSent);
			cmplCookieTableRec(db, table, stex, *TheAllReqs, phase, scope, pfx + ".sent", "Sent");
		}

		{
			CookiesStex stex("cookie.recv", "Received cookies", &StatPhaseRec::theCookiesRecv);
			cmplCookieTableRec(db, table, stex, *TheAllReps, phase, scope, pfx + ".recv", "Received");
		}

		blob << table;
	}

	if (scope.hasSide("client")) {
		XmlTable table;
		table << XmlAttr::Int("border", 0);

		{
			const String name(BlobDb::Key(pfx + ".purged.fresh", scope));
			const String title("fresh cookies evicted:");

			XmlTableRec tr;
			tr << algnLeft << XmlTableHeading(title);

			addMeasBlob(db, name, phase.stats().theCookiesPurgedFresh, "cookie", title);
			XmlTableCell cell;
			cell << db.quote(name);

			tr << cell;
			table << tr;
		}

		{
			const String name(BlobDb::Key(pfx + ".purged.stale", scope));
			const String title("stale cookies evicted:");

			XmlTableRec tr;
			tr << algnLeft << XmlTableHeading(title);

			addMeasBlob(db, name, phase.stats().theCookiesPurgedStale, "cookie", title);
			XmlTableCell cell;
			cell << db.quote(name);

			tr << cell;
			table << tr;
		}

		{
			const String name(BlobDb::Key(pfx + ".updated", scope));
			const String title("cookies updated:");

			XmlTableRec tr;
			tr << algnLeft << XmlTableHeading(title);

			addMeasBlob(db, name, phase.stats().theCookiesUpdated, "cookie", title);
			XmlTableCell cell;
			cell << db.quote(name);

			tr << cell;
			table << tr;
		}

		blob << table;
	}

	{
		XmlTag descr("description");

		XmlTextTag<XmlParagraph> p1;
		p1.buf() << "The 'HTTP Cookies' table provides statistics "
			"for HTTP cookies sent and received. The 'Messages "
			"with cookies' column shows the absolute and relative "
			"number of HTTP messages containing cookies. The "
			"'Cookies per message' column shows mininum, maximum, "
			"and mean number of cookies in HTTP messages containing "
			"cookies. The 'Total #cookies' column shows the total "
			"number of cookies sent and received.";
		descr << p1;

		blob << descr;
	}

	db << blob;
}

void SideInfo::cmplCookieTableRec(BlobDb &db, XmlTable &table, const Stex &stex, const Stex &allStex, const PhaseInfo &phase, const Scope &scope, const String &pfx, const String &name) {
	const String msgAbsoluteCountName = BlobDb::Key(pfx + ".messages.absolute", scope);
	const String msgRelativeCountName = BlobDb::Key(pfx + ".messages.relative", scope);
	const String meanName = BlobDb::Key(pfx + ".mean", scope);
	const String minName = BlobDb::Key(pfx + ".min", scope);
	const String maxName = BlobDb::Key(pfx + ".max", scope);
	const String totalCountName = BlobDb::Key(pfx + ".total", scope);
	const String msgAbsoluteCountTitle = "absolute message count";
	const String msgRelativeCountTitle = "relative message count";
	const String meanTitle = "mean";
	const String minTitle = "min";
	const String maxTitle = "max";
	const String totalCountTitle = "total count";

	if (const AggrStat *const stats = stex.partsStat(phase)) {
		addMeasBlob(db, msgAbsoluteCountName, stats->count(), "msg", msgAbsoluteCountTitle);
		if (const TmSzStat *const allStats = allStex.aggr(phase))
			addMeasBlob(db, msgRelativeCountName, Percent(stats->count(), allStats->count()), "%", msgRelativeCountTitle);
		else
			addNaMeasBlob(db, msgRelativeCountName, msgRelativeCountTitle);
		addMeasBlob(db, minName, stats->min(), "cookie", minTitle);
		addMeasBlob(db, meanName, stats->mean(), "cookie", meanTitle);
		addMeasBlob(db, maxName, stats->max(), "cookie", maxTitle);
		addMeasBlob(db, totalCountName, stats->sum(), "cookie", totalCountTitle);
	} else {
		addNaMeasBlob(db, msgAbsoluteCountName, msgAbsoluteCountTitle);
		addNaMeasBlob(db, msgRelativeCountName, msgRelativeCountTitle);
		addNaMeasBlob(db, minName, minTitle);
		addNaMeasBlob(db, meanName, meanTitle);
		addNaMeasBlob(db, maxName, maxTitle);
		addNaMeasBlob(db, totalCountName, totalCountTitle);
	}

	XmlTableRec tr;
	tr << algnLeft << XmlTableHeading(name);

	{
		XmlTableCell cell;
		cell << algnRight << db.quote(msgAbsoluteCountName);
		tr << cell;
	}

	{
		XmlTableCell cell;
		cell << algnRight << db.quote(msgRelativeCountName);
		tr << cell;
	}

	{
		XmlTableCell cell;
		cell << algnRight << db.quote(minName);
		tr << cell;
	}

	{
		XmlTableCell cell;
		cell << algnRight << db.quote(meanName);
		tr << cell;
	}

	{
		XmlTableCell cell;
		cell << algnRight << db.quote(maxName);
		tr << cell;
	}

	{
		XmlTableCell cell;
		cell << algnRight << db.quote(totalCountName);
		tr << cell;
	}

	table << tr;
}

void SideInfo::cmplObjectBlobs(BlobDb &db, const PhaseInfo &phase, const Scope &scope, const Array<Stex*> &stexes) {
	for (int s = 0; s < stexes.count(); ++s)
		cmplObjectBlob(db, *stexes[s], phase, scope);
}

void SideInfo::cmplObjectBlob(BlobDb &db, const Stex &stex, const PhaseInfo &phase, const Scope &scope) {
	const TmSzStat *aggr = stex.aggr(phase);
	if (aggr && aggr->count()) {
		const String pfx = "object." + stex.key();
		const String tlTitle = stex.name() + " stats";
		ReportBlob blob(pfx + scope, tlTitle);
		blob << XmlAttr("vprimitive", String("Object '") + stex.name() + "'");

		XmlTable table;
		table << XmlAttr::Int("border", 0);

		if (stex.parent() || &stex == TheAllReps || &stex == TheAllReqs) {
			XmlTableRec tr;
			tr << algnLeft << XmlTableHeading("contribution:");

			XmlTableCell cell;
			cell << db.include("stream." + stex.key() + ".ratio.obj" + scope);
			cell << XmlText(" by count and ");
			cell << db.include("stream." + stex.key() + ".ratio.byte" + scope);
			cell << XmlText(" by volume");
			tr << cell;

			table << tr;
		}

		{
			XmlTableRec tr;
			tr << algnLeft << XmlTableHeading("rates:");

			XmlTableCell cell;
			cell << db.include("stream." + stex.key() + ".rate" + scope);
			cell << XmlText(" or ");
			cell << db.include("stream." + stex.key() + ".bwidth" + scope);
			tr << cell;

			table << tr;
		}

		{
			XmlTableRec tr;
			tr << algnLeft << XmlTableHeading("totals:");

			XmlTableCell cell;
			cell << db.include("stream." + stex.key() + ".size.count" + scope);
			cell << XmlText(" and ");
			cell << db.include("stream." + stex.key() + ".size.sum" + scope);
			tr << cell;

			table << tr;
		}

		{
			XmlTableRec tr;
			tr << algnLeft << XmlTableHeading("response time:");

			XmlTableCell cell;
			cell << db.include("object." + stex.key() + ".rptm.min" + scope);
			cell << XmlText(" min, ");
			cell << db.include("object." + stex.key() + ".rptm.mean" + scope);
			cell << XmlText(" mean, and ");
			cell << db.include("object." + stex.key() + ".rptm.max" + scope);
			cell << XmlText(" max");
			tr << cell;

			table << tr;
		}

		{
			XmlTableRec tr;
			tr << algnLeft << XmlTableHeading("response size:");

			XmlTableCell cell;
			cell << db.include("object." + stex.key() + ".size.min" + scope);
			cell << XmlText(" min, ");
			cell << db.include("object." + stex.key() + ".size.mean" + scope);
			cell << XmlText(" mean, and ");
			cell << db.include("object." + stex.key() + ".size.max" + scope);
			cell << XmlText(" max");
			tr << cell;

			table << tr;
		}

		blob << table;

		if (stex.hist(phase)) {
			{
				RptmHistFig fig;
				fig.configure(pfx + ".rptm.fig" + scope, "response time distribution");
				fig.stats(&stex, &phase);
				blob << db.include(fig.plot(db).key());
			}

			{
				SizeHistFig fig;
				fig.configure(pfx + ".size.fig" + scope, "object size distribution");
				fig.stats(&stex, &phase);
				blob << db.include(fig.plot(db).key());
			}
		} else {
			blob << XmlTextTag<XmlParagraph>("No response time and size "
				"histograms were collected or stored for this object class.");	
		}

		if (stex.trace(phase.availStats())) {
			LoadTraceFig figLoad;
			TmSzLoadStex loadStex(&stex);
			figLoad.configure(pfx + ".load.trace" + scope, "load trace");
			figLoad.stats(&loadStex, &this->phase(scope));
			figLoad.globalStart(theTest->startTime());
			const String &figLoadKey = figLoad.plot(db).key();
			blob << db.include(figLoadKey);

			RptmTraceFig figRptm;
			figRptm.configure(pfx + ".rptm.trace" + scope, "response time trace");
			figRptm.stats(&stex, &this->phase(scope));
			figRptm.globalStart(theTest->startTime());
			const String &figRptmKey = figRptm.plot(db).key();
			blob << db.include(figRptmKey);
		} else {
			blob << XmlTextTag<XmlParagraph>("No response time and size "
				"traces are collected for this object class.");
		}

		{
			XmlTag descr("description");
			XmlNodes nodes;
			stex.describe(nodes);
			descr << nodes;
			blob << descr;
		}

		db.add(blob);
	} else
	if (!stex.ignoreUnseen())
		theUnseenObjects.push_back(stex.name());
}

void SideInfo::cmplUnseenObjectsBlob(BlobDb &db, const Scope &scope) {
	if (theUnseenObjects.empty())
		return;

	ReportBlob blob(BlobDb::Key("unseen_objects", scope), "Unseen objects");
	blob << XmlAttr("vprimitive", String("Unseen objects"));

	{
		XmlTextTag<XmlParagraph> p;
		p.buf() << "No instances of these objects were observed or "
			<< "recorded in the given scope:";
		for (std::list<String>::const_iterator i = theUnseenObjects.begin();
			i != theUnseenObjects.end();
			++i)
			p.buf() << (i == theUnseenObjects.begin() ? " " : ", ") << *i;
		p.buf() << '.';
		blob << p;
	}

	{
		XmlTextTag<XmlParagraph> p;
		p.buf() << "An object unseen in one scope may be present in "
			<< "another scope.";
		blob << p;
	}

	db << blob;
}

void SideInfo::cmplSideSum(BlobDb &db) {
	ReportBlob blob(BlobDb::Key("summary", execScope()), "test side summary");
	blob << db.quote("load" + execScope());
	blob << db.quote("hit.ratio" + execScope());
	blob << db.quote("reply_stream.table" + execScope());
	blob << db.quote("reply_object.table" + execScope());
	db << blob;
}


void SideInfo::AddStex(Array<Stex*> &stexes, Stex *stex, const Stex *parent) {
	Assert(stex);
	if (parent && Should(stex != parent))
		stex->parent(parent);

	stexes.append(stex);
}

void SideInfo::Configure() {
	TheAllReps = new AllRepsStex("rep", "all replies");

	Stex *allContTypes = new AllContTypesStex("cont_type_all", "all response content types", &StatPhaseRec::theRepContType);
	for (int i = 0; i < ContTypeStat::Kinds().count(); ++i) {
		char buf[128];
		ofixedstream s(buf, sizeof(buf));
		s << "rep_cont_type_" << i << ends;
		const String key = buf;
		const String &cname = *ContTypeStat::Kinds()[i];
		const String name = String("\"") + cname + "\" response";
		if (cname[0] != '_')
			AddStex(TheReplyStex, new ContTypeStex(key, name, i, &StatPhaseRec::theRepContType), allContTypes);
	}
	AddStex(TheReplyStex, allContTypes, TheAllReps);

	Stex *hitsAndMisses = new HitMissesStex("hits_and_misses", "hits and misses");
	AddStex(TheReplyStex, new HitsStex("hits", "hits"), hitsAndMisses);
	AddStex(TheReplyStex, new MissesStex("misses", "misses"), hitsAndMisses);
	AddStex(TheReplyStex, hitsAndMisses, TheAllReps);

	Stex *allIms = new ValidationHitMissStex("ims_scAll", "all ims", &StatPhaseRec::theImsXacts, &StatIntvlRec::theIms);
	AddStex(TheReplyStex, new ValidationHitStex("ims_sc200", "ims/200", &StatPhaseRec::theImsXacts), allIms);
	AddStex(TheReplyStex, new ValidationMissStex("ims_sc304", "ims/304", &StatPhaseRec::theImsXacts), allIms);
	AddStex(TheReplyStex, allIms, TheAllReps);

	Stex *allCachable = new AllCachableStex("all_cachable", "cachable and not");
	AddStex(TheReplyStex, new CachableStex("cachable", "cachable"), allCachable);
	AddStex(TheReplyStex, new UnCachableStex("uncachable", "not cachable"), allCachable);
	AddStex(TheReplyStex, allCachable, TheAllReps);

	Stex *allFtp = new ValidationHitMissStex("ftp_all_modes", "FTP all modes", &StatPhaseRec::theFtpXacts, 0);
	AddStex(TheReplyStex, new ValidationHitStex("ftp_active", "FTP active", &StatPhaseRec::theFtpXacts), allFtp);
	AddStex(TheReplyStex, new ValidationMissStex("ftp_passive", "FTP passive", &StatPhaseRec::theFtpXacts), allFtp);
	AddStex(TheReplyStex, allFtp, TheAllReps);

	AddStex(TheReplyStex, new FillStex("fill", "fill"), TheAllReps);

	AddStex(TheReplyStex, new SimpleStex("reload", "reload",
		&StatPhaseRec::theReloadXacts, &StatIntvlRec::theReload), TheAllReps);
	AddStex(TheReplyStex, new SimpleStex("range", "range",
		&StatPhaseRec::theRangeXacts, &StatIntvlRec::theRange), TheAllReps);
	AddStex(TheReplyStex, new SimpleStex("abort", "abort",
		0, &StatIntvlRec::theAbort), TheAllReps);
	AddStex(TheReplyStex, new SimpleStex("redir_req", "redirected request",
		&StatPhaseRec::theRediredReqXacts, &StatIntvlRec::theRediredReq), TheAllReps);
	AddStex(TheReplyStex, new SimpleStex("rep_to_redir", "reply to redirect",
		&StatPhaseRec::theRepToRedirXacts, &StatIntvlRec::theRepToRedir), TheAllReps);

	Stex *allMethods = new AllMethodsStex("method_all", "all non-gets");
	AddStex(TheReplyStex, new SimpleStex("method_head", "HEAD",
		&StatPhaseRec::theHeadXacts, &StatIntvlRec::theHead), allMethods);
	AddStex(TheReplyStex, new SimpleStex("method_post", "POST",
		&StatPhaseRec::thePostXacts, &StatIntvlRec::thePost), allMethods);
	AddStex(TheReplyStex, new SimpleStex("method_put", "PUT",
		&StatPhaseRec::thePutXacts, &StatIntvlRec::thePut), allMethods);
	AddStex(TheReplyStex, new SimpleStex("method_connect", "CONNECT",
		&StatPhaseRec::theConnectXacts, &StatIntvlRec::theConnect), allMethods);
	AddStex(TheReplyStex, allMethods, TheAllReps);

	AddStex(TheReplyStex, TheAllReps, 0);

	AddStex(TheReplyStex, new SimpleStex("page", "page",
		&StatPhaseRec::thePageHist, &StatIntvlRec::thePage), 0);

	TheAllReqs = new AllContTypesStex("req_cont_type_all", "all request content types", &StatPhaseRec::theReqContType);
	for (int i = 0; i < ContTypeStat::Kinds().count(); ++i) {
		char buf[128];
		ofixedstream s(buf, sizeof(buf));
		s << "req_cont_type_" << i << ends;
		const String key = buf;
		const String &cname = *ContTypeStat::Kinds()[i];
		const String name = String("\"") + cname + "\" request";
		if (cname[0] != '_')
			AddStex(TheRequestStex, new ContTypeStex(key, name, i, &StatPhaseRec::theReqContType), TheAllReqs);
	}
	AddStex(TheRequestStex, TheAllReqs, 0);

	Stex *proxyValidation = new ValidationHitMissStex("all_proxy_validations", "all proxy validations", &StatPhaseRec::theProxyValidationR, &StatIntvlRec::theProxyValidations);
	TheUsefulProxyValidation = new ValidationHitStex("useful_proxy_validations", "useful proxy validations", &StatPhaseRec::theProxyValidationR);
	AddStex(TheReplyStex, TheUsefulProxyValidation, proxyValidation);
	AddStex(TheReplyStex, new ValidationMissStex("useless_proxy_validations", "useless proxy validations", &StatPhaseRec::theProxyValidationR), proxyValidation);
	AddStex(TheReplyStex, proxyValidation, TheAllReps);

	AddProtoStexes(&StatIntvlRec::theSocksStat);
	AddProtoStexes(&StatIntvlRec::theSslStat);
	AddProtoStexes(&StatIntvlRec::theFtpStat);

	/* auth related stexes */
	{
		Stex *const authNone = new SimpleStex("auth.none", "no auth",	0, &StatIntvlRec::theAuthNone);
		AddStex(TheAuthStex, authNone, TheAllReps);
		AddStex(TheReplyStex, authNone, 0);
	}
	{
		Stex *const authIngBasic = new AuthIngStex("auth.ing.basic", "Basic auth-ing", AuthPhaseStat::sBasic);
		AddStex(TheAuthStex, authIngBasic, TheAllReps);
	}
	{
		Stex *const authIngNtlm = new AuthIngStex("auth.ing.ntlm", "NTLM auth-ing", AuthPhaseStat::sNtlm);
		AddStex(TheAuthStex, authIngNtlm, TheAllReps);
	}
	{
		Stex *const authIngNegotiate = new AuthIngStex("auth.ing.negotiate", "Negotiate auth-ing", AuthPhaseStat::sNegotiate);
		AddStex(TheAuthStex, authIngNegotiate, TheAllReps);
	}
	{
		Stex *const authIngAny = new AllAuthIngStex("auth.ing.any", "all auth-ing");
		AddStex(TheAuthStex, authIngAny, TheAllReps);
	}
	{
		Stex *const authEdBasic = new AuthEdStex("auth.ed.basic", "Basic auth-ed", AuthPhaseStat::sBasic);
		AddStex(TheAuthStex, authEdBasic, TheAllReps);
	}
	{
		Stex *const authEdNtlm = new AuthEdStex("auth.ed.ntlm", "NTLM auth-ed", AuthPhaseStat::sNtlm);
		AddStex(TheAuthStex, authEdNtlm, TheAllReps);
	}
	{
		Stex *const authEdNegotiate = new AuthEdStex("auth.ed.negotiate", "Negotiate auth-ed", AuthPhaseStat::sNegotiate);
		AddStex(TheAuthStex, authEdNegotiate, TheAllReps);
	}
	{
		Stex *const authFtp = new AuthIngStex("auth.ftp", "FTP auth", AuthPhaseStat::sFtp);
		AddStex(TheAuthStex, authFtp, TheAllReps);
	}
	{
		Stex *const authEdAny = new AllAuthEdStex("auth.ed.any", "all auth-ed");
		AddStex(TheAuthStex, authEdAny, TheAllReps);
	}
	{
		Stex *const authAny = new AllAuthStex("auth.any", "all auth");
		AddStex(TheAuthStex, authAny, TheAllReps);
		AddStex(TheReplyStex, authAny, 0);
	}
	{
		Stex *const tunneled = new SimpleStex("tunneled", "tunneled", 0, &StatIntvlRec::theTunneled);
		AddStex(TheAuthStex, tunneled, TheAllReps);
		AddStex(TheReplyStex, tunneled, 0);
	}
	AddStex(TheAuthStex, TheAllReps, 0);

	AddStex(TheCompoundReplyStex, new CompoundReplyStex("compound.auth.basic.rep", "Basic auth replies",
		&StatPhaseRec::theAuthBasic), TheAllReps);
	AddStex(TheCompoundReplyStex, new CompoundReplyStex("compound.auth.ntlm.rep", "NTLM auth replies",
		&StatPhaseRec::theAuthNtlm), TheAllReps);
	AddStex(TheCompoundReplyStex, new CompoundReplyStex("compound.auth.negotiate.rep", "Negotiate auth replies",
		&StatPhaseRec::theAuthNegotiate), TheAllReps);
	AddStex(TheCompoundReplyStex, new AllCompoundRepsStex("compound.any.reps",
		"all compound replies"), TheAllReps);
	AddStex(TheCompoundReplyStex, new CompoundReplyStex("compound.auth.not.rep", "isolated replies",
		&StatPhaseRec::theIsolated), TheAllReps);

	AddStex(TheCompoundRequestStex, new CompoundRequestStex("compound.auth.basic.req", "Basic auth requests",
		&StatPhaseRec::theAuthBasic), TheAllReqs);
	AddStex(TheCompoundRequestStex, new CompoundRequestStex("compound.auth.ntlm.req", "NTLM auth requests",
		&StatPhaseRec::theAuthNtlm), TheAllReqs);
	AddStex(TheCompoundRequestStex, new CompoundRequestStex("compound.auth.negotiate.req", "Negotiate auth requests",
		&StatPhaseRec::theAuthNegotiate), TheAllReqs);
	AddStex(TheCompoundRequestStex, new AllCompoundReqsStex("compound.any.reqs",
		"all compound requests"), TheAllReqs);
	AddStex(TheCompoundRequestStex, new CompoundRequestStex("compound.auth.not.req", "isolated requests",
		&StatPhaseRec::theIsolated), TheAllReqs);

	AddStex(TheReplyStex, new SimpleStex("custom_stats", "custom",
		&StatPhaseRec::theCustomXacts, &StatIntvlRec::theCustom), 0);

	Stex *allReplyStatus = new AllStatusCodeStex("reply_status.all", "All HTTP reply status codes", &StatPhaseRec::theStatusCode);
	for (int i = 0; i <= StatusCodeStat::scsMaxValue; ++i) {
		const String label(AnyToString(i));
		const String name("reply_status." + label);
		const String title("HTTP reply status code " + label);
		AddStex(TheReplyStatusStex,
			new StatusCodeStex(name, title, &StatPhaseRec::theStatusCode, i),
			allReplyStatus);
	}
	AddStex(TheReplyStatusStex,
		new StatusCodeStex("reply_status.other", "Other HTTP reply status code", &StatPhaseRec::theStatusCode, RepHdr::scUnknown),
		allReplyStatus);
	AddStex(TheReplyStatusStex, allReplyStatus, 0);
}

void SideInfo::AddProtoStexes(ProtoIntvlPtr protoPtr) {
	StatIntvlRec dummy;
	const ProtoIntvlStat protoStat = dummy.*protoPtr;
	const String pfx = protoStat.id();
	const String protoName = protoStat.name();

	Stex *hitsAndMisses = new ProtoHitMissesStex(protoPtr, pfx + "_hits_and_misses", protoName + " hits and misses");
	AddStex(TheReplyStex, new ProtoHitsStex(protoPtr, pfx + "_hits", protoName + " hits"), hitsAndMisses);
	AddStex(TheReplyStex, new ProtoMissesStex(protoPtr, pfx + "_misses", protoName + " misses"), hitsAndMisses);
	AddStex(TheReplyStex, hitsAndMisses, TheAllReps);
}

// sort stexes using cmpByCountContrib
void SideInfo::SortStexes(const PhaseInfo &phase, const Array<Stex*> &in, Array<Stex*> &out) {
	out.stretch(in.count());
	// insertion sort keeps original array constant and needs no cmp wrapper
	for (int i = 0; i < in.count(); ++i) {
		Stex *const stex = in[i];
		bool inserted = false;
		for (int j = 0; j < out.count(); ++j) {
			if (stex->cmpByCountContrib(phase, *out[j]) < 0) {
				out.insert(stex, j);
				inserted = true;
				break;
			}
		}
		if (!inserted)
			out.append(stex);
	}
}
