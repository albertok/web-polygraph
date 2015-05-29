
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "xstd/gadgets.h"
#include "base/polyLogCats.h"
#include "base/polyVersion.h"
#include "base/AnyToString.h"
#include "runtime/HttpDate.h"
#include "xml/XmlAttr.h"
#include "xml/XmlNodes.h"
#include "xml/XmlTable.h"
#include "xml/XmlSection.h"
#include "xml/XmlParagraph.h"
#include "xml/XmlText.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/HistStex.h"
#include "loganalyzers/HistogramFigure.h"
#include "loganalyzers/InfoScopes.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/SideInfo.h"
#include "loganalyzers/TestInfo.h"

static XmlAttr algnLeft("align", "left");
static XmlAttr algnRight("align", "right");


TestInfo::TestInfo(const String &aLabel): theLabel(aLabel), theSides(lgcEnd) {
	theSides.count(lgcEnd);
	theSides[lgcCltSide] = new SideInfo(lgcCltSide);
	theSides[lgcCltSide]->test(this);
	theSides[lgcSrvSide] = new SideInfo(lgcSrvSide);
	theSides[lgcSrvSide]->test(this);
	theExecScope.addSide("client");
	theExecScope.addSide("server");
	theExecScope.name("baseline");
}

TestInfo::~TestInfo() {
	while (theSides.count()) {
		if (theSides.last())
			theSides.last()->test(0);
		delete theSides.pop();
	}

	while (theScopes.count()) delete theScopes.pop();
}

void TestInfo::execScope(const Scope &aScope) {
	theExecScope = aScope;
}

const TestInfo::Scope &TestInfo::guessExecScope() {
	Assert(!theExecScope);
	const SideInfo &side = aSide();

	// find last phase with peak (highest) request rate
	String bestName;
	double peakRate = -1;
	String allBestName;
	double allPeakRate = -1;
	for (int i = 0; i < side.phaseCount(); ++i) {
		const PhaseInfo &phase = side.phase(i);
		const double rate = phase.availStats().reqRate();
		// allow for 1% rate diff among phases with the same configured rate
		if (phase.hasStats())
			if (!bestName || peakRate <= 1.01*rate) {
				peakRate = rate;
				bestName = phase.name();
			}
		if (!bestName)
			if (!allBestName || allPeakRate <= 1.01*rate) {
				allPeakRate = rate;
				allBestName = phase.name();
			}
		if (phase.stats().primary)
			theExecScope.addPhase(phase.name());
	}
	if (!bestName)
		bestName = allBestName;

	if (!theExecScope && Should(bestName)) {
		clog << "no primary or 'executive summary' phases specified, using '"
			<< bestName << "' phase" << endl;
		theExecScope.addPhase(bestName);
	}

	return theExecScope;
}

const String &TestInfo::label() const {
	return theLabel;
}

const String &TestInfo::pglCfg() const {
	return thePglCfg;
}

Time TestInfo::startTime() const {
	return theStartTime;
}

const InfoScope &TestInfo::execScope() const {
	return theExecScope;
}

const SideInfo *TestInfo::cltSideExists()  const {
	return side(lgcCltSide).procCount() ? theSides[lgcCltSide] : 0;
}

const SideInfo *TestInfo::srvSideExists() const {
	return side(lgcSrvSide).procCount() ? theSides[lgcSrvSide] : 0;
}

SideInfo &TestInfo::cltSide() {
	return side(lgcCltSide);
}

SideInfo &TestInfo::srvSide() {
	return side(lgcSrvSide);
}

SideInfo &TestInfo::side(int logCat) {
	Assert(logCat == lgcCltSide || logCat == lgcSrvSide);
	Assert(theSides[logCat]);
	return *theSides[logCat];
}

const SideInfo &TestInfo::aSide() const {
	return cltSideExists() ? cltSide() : srvSide();
}

const SideInfo &TestInfo::cltSide() const {
	return side(lgcCltSide);
}

const SideInfo &TestInfo::srvSide() const {
	return side(lgcSrvSide);
}

const SideInfo &TestInfo::side(int logCat) const {
	Assert(logCat == lgcCltSide || logCat == lgcSrvSide);
	Assert(theSides[logCat]);
	return *theSides[logCat];
}

int TestInfo::scopes(InfoScopes &res) const {
	if (!twoSided())
		return aSide().scopes(res);

	for (int i = 0; i < theScopes.count(); ++i)
		res.add(*theScopes[i]);

	return res.count();
}

void TestInfo::checkCommonPglCfg() {
	if (!cltSideExists() && srvSideExists())
		thePglCfg = srvSide().pglCfg();
	else
	if (cltSideExists() && !srvSideExists())
		thePglCfg = cltSide().pglCfg();
	else
	if (cltSide().pglCfg() == srvSide().pglCfg())
		thePglCfg = cltSide().pglCfg();
	else
		cerr << label() << ": warning: client- and server-side PGL configurations differ" << endl;
}

void TestInfo::checkCommonBenchmarkVersion() {
	if (!cltSideExists() && srvSideExists())
		theBenchmarkVersion = srvSide().benchmarkVersion();
	else
	if (cltSideExists() && !srvSideExists())
		theBenchmarkVersion = cltSide().benchmarkVersion();
	else
	if (cltSide().benchmarkVersion() == srvSide().benchmarkVersion()) {
		theBenchmarkVersion = cltSide().benchmarkVersion();
	}
	else {
		cerr << label() << ": warning: client- and server-side"
			<< " benchmark versions differ" << endl;
		theBenchmarkVersion = String();
	}
}

void TestInfo::checkCommonStartTime() {

	if (!cltSideExists() && srvSideExists())
		theStartTime = srvSide().startTime();
	else
	if (cltSideExists() && !srvSideExists())
		theStartTime = cltSide().startTime();
	else 
	if (cltSide().startTime() >= 0 && srvSide().startTime() >= 0) {
		const Time diff = 
			Max(cltSide().startTime(), srvSide().startTime()) -
			Min(cltSide().startTime(), srvSide().startTime());
		if (diff > Time::Sec(5*60))
			cerr << label() << ": warning: client- and server-side process "
				<< "start times differ by " << diff << endl;
		// should we not set a start time in this case (like with thePglCfg)
		theStartTime = Min(cltSide().startTime(), srvSide().startTime());
	}
}

void TestInfo::checkConsistency() {
	if (!cltSideExists() && !srvSideExists()) {
		cerr << "no client- or server-side information found in the logs, exiting" 
			<< endl << xexit;
	}

	if (!cltSideExists() || !srvSideExists()) {
		const String &sname = cltSideExists() ?
			cltSide().name() : srvSide().name();
		theOneSideWarn = String("Only ") + sname + 
			"-side information found in the logs.";
		cerr << "warning: " << theOneSideWarn
			<< "  The report will be incomplete and less accurate" << endl;
	}

	if (cltSideExists())
		cltSide().checkConsistency();
	if (srvSideExists())
		srvSide().checkConsistency();

	checkCommonBenchmarkVersion();
	checkCommonPglCfg();
	checkCommonStartTime();
	//checkCommonPhases();
}

int TestInfo::repCount(const Scope &scope) const {
	return cltSideExists() ? cltSide().repCount(scope) : -1;
}

int TestInfo::hitCount(const Scope &scope) const {
	return twoSided() ? cltSide().repCount(scope) - srvSide().repCount(scope) : -1;
}

int TestInfo::uselessProxyValidationCount(const Scope &scope) const {
	return srvSideExists() ? srvSide().uselessProxyValidationCount(scope) : -1;
}

BigSize TestInfo::repVolume(const Scope &scope) const {
	return cltSideExists() ? cltSide().repVolume(scope) : BigSize();
}

BigSize TestInfo::hitVolume(const Scope &scope) const {
	return twoSided() ? 
		cltSide().repVolume(scope) - srvSide().repVolume(scope) :
		BigSize();
}

BigSize TestInfo::uselessProxyValidationVolume(const Scope &scope) const {
	return srvSideExists() ? srvSide().uselessProxyValidationVolume(scope) : BigSize();
}

AggrStat TestInfo::lastReqByteWritten(const Scope &scope) const {
	return cltSideExists() ? cltSide().lastReqByteWritten(scope) : AggrStat();
}

AggrStat TestInfo::lastReqByteRead(const Scope &scope) const {
	return srvSideExists() ? srvSide().lastReqByteRead(scope) : AggrStat();
}

AggrStat TestInfo::firstRespByteWritten(const Scope &scope) const {
	return srvSideExists() ? srvSide().firstRespByteWritten(scope) : AggrStat();
}

AggrStat TestInfo::firstRespByteRead(const Scope &scope) const {
	return cltSideExists() ? cltSide().firstRespByteRead(scope) : AggrStat();
}

void TestInfo::cmplExecSumVars(BlobDb &db) {
	static const String tlLabel = "test label";
	addMeasBlob(db, "label", theLabel, "string", tlLabel);

	{
		ostringstream buf;
		HttpDatePrint(buf, startTime());
		buf << ends;
		static const String tlStartTime = "test start time";
		addMeasBlob(db, "start.time", buf.str().c_str(), "string", tlStartTime);
		streamFreeze(buf, false);
	}

	{
		static const String tlTitle = "benchmark software version";
		ReportBlob blob("benchmark.version" + theExecScope, tlTitle);
		if (theBenchmarkVersion) {
			blob << XmlText(theBenchmarkVersion);
		} else {
			XmlParagraph p;
			p << XmlText("cannot show a single benchmark version because ");
			p << db.ptr(BlobDb::Key("benchmark.version", execScope().oneSide("client")), XmlText("client-"));
			p << XmlText(" and ");
			p << db.ptr(BlobDb::Key("benchmark.version", execScope().oneSide("server")), XmlText("server-side"));
			p << XmlText(" versions differ");
			blob << p;
		}
		db << blob;
	}

	{
		static const String tlTitle = "reporter software version";
		ReportBlob blob("reporter.version" + theExecScope, tlTitle);
		blob << XmlText(PolyVersion());
		db << blob;
	}
}

void TestInfo::cmplExecSum(BlobDb &db) {
	const Scope &cltScope = theExecScope.oneSide("client");

	cmplExecSumTable(db, cltScope);
	cmplExecSumPhases(db, cltScope);
}

void TestInfo::cmplExecSumTable(BlobDb &db, const Scope &cltScope) {
	static const String tlTitle = "executive summary table";
	ReportBlob blob("summary.exec.table" + theExecScope, tlTitle);
	blob << XmlAttr("vprimitive", "Test summary table");

	XmlTable table;
	table << XmlAttr::Int("border", 0);

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("label:");

		XmlTableCell cell;
		cell << db.include("label");
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("throughput:");

		XmlTableCell cell;
		cell << db.quote("rep.rate" + cltScope);
		cell << XmlText(" or ");
		cell << db.quote("rep.bwidth" + cltScope);
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("response time:");

		XmlTableCell cell;
		//cell << db.quote("object.hits.rptm.mean" + cltScope);
		//cell << XmlText(" hit, ");
		cell << db.quote("rep.rptm.mean" + cltScope);
		cell << XmlText(" mean");
		//cell << db.quote("object.misses.rptm.mean" + cltScope);
		//cell << XmlText(" miss");
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("hit ratios:");

		XmlTableCell cell;
		cell << db.quote("hit.ratio.obj" + theExecScope);
		cell << XmlText(" DHR and ");
		cell << db.quote("hit.ratio.byte" + theExecScope);
		cell << XmlText(" BHR");
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("unique URLs:");

		XmlTableCell cell;
		cell << db.quote("url.unique.count" + cltScope);
		cell << XmlText(" (");
		cell << db.quote("url.recurrence.ratio" + cltScope);
		cell << XmlText(" recurrence)");
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("errors:");

		XmlTableCell cell;
		cell << db.quote("xact.error.ratio" + cltScope);
		cell << XmlText(" (");
		cell << db.quote("xact.error.count" + cltScope);
		cell << XmlText(" out of ");
		cell << db.quote("xact.count" + cltScope);
		cell << XmlText(")");
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("duration:");

		XmlTableCell cell;
		cell << db.include("duration" + cltScope);
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("start time:");

		XmlTableCell cell;
		cell << db.include("start.time");
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("workload:");

		XmlTableCell cell;
		cell << db.ptr("workload" + theExecScope, XmlText("available"));
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("Polygraph version:");

		XmlTableCell cell;
		cell << db.include("benchmark.version" + theExecScope);
		tr << cell;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("reporter version:");

		XmlTableCell cell;
		cell << db.include("reporter.version" + theExecScope);
		tr << cell;

		table << tr;
	}

	blob << table;
	db << blob;
}

void TestInfo::cmplExecSumPhases(BlobDb &db, const Scope &cltScope) {
	static const String tlTitle = "executive summary phases";
	ReportBlob blob("summary.exec.phases" + theExecScope, tlTitle);
	blob << XmlAttr("vprimitive", "Test summary phases");

	XmlParagraph p;
	XmlText text;
	text.buf() << "This executive summary and baseline report statistics"
		<< " are based on the following "
		<< theExecScope.phases().count() << " test phase(s): ";

	{for (int i = 0; i < theExecScope.phases().count(); ++i) {
		if (i)
			text.buf() << ", ";
		text.buf() << *theExecScope.phases().item(i);
	}}

	text.buf() << ". The test has the following " 
		<< aSide().phaseCount() << " phase(s): ";

	{for (int i = 0; i < aSide().phaseCount(); ++i) {
		if (i)
			text.buf() << ", ";
		text.buf() << aSide().phase(i).name();
	}}
	text.buf() << '.';

	p << text;
	blob << p;
	db << blob;
}

void TestInfo::cmplWorkload(BlobDb &db) {
	static const String tlTitle = "test workload";
	ReportBlob blob(BlobDb::Key("workload", theExecScope), tlTitle);

	if (!thePglCfg) {
		XmlParagraph p;
		p << XmlText("Cannot show a single test workload because ");
		p << db.ptr(BlobDb::Key("workload.code", execScope().oneSide("client")), XmlText("client-"));
		p << XmlText(" and ");
		p << db.ptr(BlobDb::Key("workload.code", execScope().oneSide("server")), XmlText("server-side"));
		p << XmlText(" PGL configurations differ.");
		p << db.reportNote("workload", db.ptr("page.workload", XmlText("client- and server-side PGL configurations differ")));
		blob << p;
	}

	{
		XmlSection sect("English interpretation");
		sect << XmlTextTag<XmlParagraph>("TBD.");
		blob << sect;
	}

	if (thePglCfg)
		cmplWorkloadBlob(blob, "", thePglCfg);
	else {
		cmplWorkloadBlob(blob, "client", cltSide().pglCfg());
		cmplWorkloadBlob(blob, "server", srvSide().pglCfg());
	}

	db << blob;
}

void TestInfo::cmplWorkloadBlob(ReportBlob &blob, const String &side, const String &pglCfg) {
	{
		const String tlPglTitle((side ? side + "-side " : String()) + "PGL code");

		XmlSection sect(tlPglTitle);

		ReportBlob code(BlobDb::Key("workload.code",
			side ? execScope().oneSide(side) : theExecScope),
			tlPglTitle);
		XmlTag codesample("codesample");
		codesample << XmlText(pglCfg);
		code << codesample;

		sect << code;
		blob << sect;
	}
}

void TestInfo::cmplHitRatioVars(BlobDb &db, const Scope &scope) {
	if (twoSided()) {
		const String sfx = BlobDb::KeySuffix(scope);
		const double dhr = Percent(hitCount(scope), repCount(scope));
		static const String tlDhr = "document hit ratio";
		addMeasBlob(db, "hit.ratio.obj" + sfx, dhr, "%", tlDhr);
		const double bhr = Percent(hitVolume(scope).byted(), repVolume(scope).byted());
		static const String tlBhr = "byte hit ratio";
		addMeasBlob(db, "hit.ratio.byte" + sfx, bhr, "%", tlBhr);
	} else {
		// XXX: put err pointer to the theOneSideWarn-based description
		Should(false);
	}
}

void TestInfo::cmplHitRatio(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "hit ratios";
	ReportBlob blob(BlobDb::Key("hit.ratio", scope), tlTitle);
	blob << XmlAttr("vprimitive", "Hit Ratios");

	if (twoSided()) {

		cmplHitRatioTable(db, blob, scope);

		{
			XmlTag descr("description");

			XmlTextTag<XmlParagraph> p1;
			p1.buf() << "The hit ratios table shows measured hit "
				<< "ratios. Hits are calculated based on client- and "
				<< "server-side traffic comparison. Offered hits are "
				<< "counted for 'basic' transactions only (simple HTTP GET "
				<< "requests with '200 OK' responses). Measured hit stats "
				<< "are based on all transactions. Thus, 'offered' hit ratio "
				<< "are not the same as 'ideal' hit ratio in this context. ";
			descr << p1;

			XmlTextTag<XmlParagraph> p2;
			p2.buf() << "Measured hit count or volume is the difference "
				<< "between client- and server-side traffic counts or "
				<< "volumes. "
				<< "DHR, Document Hit Ratio, is the ratio of the total "
				<< "number of hits to the number of all transactions. "
				<< "BHR, Byte Hit Ratio, is the ratio of "
				<< "the total volume (a sum of response sizes) of hits to the "
				<< "total volume of all transactions. "
				<< "Negative measured hit ratios are possible if server-side "
				<< "traffic of a cache exceeds client-side traffic (e.g., "
				<< "due to optimistic prefetching or extra freshness checks) "
				<< "and if side measurements are out-of-sync. "
				<< "Negative measured BHR can also be due to "
				<< "aborted-by-robots transactions.";
			descr << p2;

			XmlParagraph p3;
			p3 << XmlText("A less accurate way to measure hit ratio is to "
				"detect hits on the client-side using custom HTTP headers. "
				"A hit ratio table based on client-side tricks is available ");
			p3 << db.ptr("hit.ratio" + scope.oneSide("client"), XmlText("elsewhere"));
			p3 << XmlText(".");
			descr << p3;

			blob << descr;
		}

	} else {
		XmlParagraph para;
		para << XmlText(theOneSideWarn);
		if (cltSideExists()) {
			para << XmlText("  See ");
			para << db.ptr("summary" + theExecScope.oneSide("client"), XmlText("client-side"));
			para << XmlText(" information for hit ratio estimations (if any)");
		} else {
			para << XmlText("  No hit ratio measurements");
			para << XmlText(" can be derived from server-side logs.");
		}
		blob << para;
	}

	db << blob;
}

void TestInfo::cmplHitRatioTable(BlobDb &db, XmlTag &parent, const Scope &scope) {
	Assert(twoSided());

	static const String tlTitle = "hit ratio table";
	ReportBlob blob(BlobDb::Key("hit.ratio.table", scope), tlTitle);
	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Hit Ratios");

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
		tr << algnLeft << XmlTableHeading("measured");

		XmlTableCell dhr;
		dhr << algnRight
			<< db.quote("hit.ratio.obj" + scope);
		tr << dhr;

		XmlTableCell bhr;
		bhr << algnRight
			<< db.quote("hit.ratio.byte" + scope);
		tr << bhr;

		table << tr;
	}

	blob << table;
	db << blob;
	parent << blob;
}

void TestInfo::cmplCheapProxyValidationVars(BlobDb &db, const Scope &scope) {
	if (twoSided()) {
		const String sfx = BlobDb::KeySuffix(scope);
		const double dhr = Percent(hitCount(scope) + uselessProxyValidationCount(scope), repCount(scope));
		static const String tlDhr = "cheap proxy validation document hit ratio";
		addMeasBlob(db, "cheap_proxy_validation.ratio.obj" + sfx, dhr, "%", tlDhr);
		const double bhr = Percent(hitVolume(scope).byted() + uselessProxyValidationVolume(scope).byted(), repVolume(scope).byted());
		static const String tlBhr = "cheap proxy validation byte hit ratio";
		addMeasBlob(db, "cheap_proxy_validation.ratio.byte" + sfx, bhr, "%", tlBhr);
	} else {
		// XXX: put err pointer to the theOneSideWarn-based description
		Should(false);
	}
}

void TestInfo::cmplCheapProxyValidation(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "cheap proxy validation";
	ReportBlob blob(BlobDb::Key("cheap_proxy_validation.ratio", scope), tlTitle);
	blob << XmlAttr("vprimitive", "Cheap proxy validation");

	if (twoSided()) {

		cmplCheapProxyValidationTable(db, blob, scope);

		{
			XmlTag descr("description");

			XmlTextTag<XmlParagraph> p1;
			p1.buf() << "The 'Cheap proxy validation' table is similar to "
				"'Hit Ratios' table. But it reflects hit ratios "
				"that would have been observed if successful "
				"(i.e., useless or in vein) validation initiated "
				"by the proxy were so 'cheap' that they could have "
				"been ignored rather than decrease measured hit ratio. "
				"The following formula is used to calculate "
				"cheap proxy validation hit ratio:";
			descr << p1;

			XmlTextTag<XmlParagraph> p2;
			p2.buf() << "cheap_proxy_validation_hits = "
				"all_client_side_responses "
				"- all_server_side_responses "
				"+ useless_server_side_proxy_validation_responses";
			descr << p2;

			XmlTextTag<XmlParagraph> p3;
			p3.buf() << "cheap_proxy_validation_HR = "
				"cheap_proxy_validation_hits "
				"/ all_client_side_responses";
			descr << p3;

			const String all_proxy_validations_key =
				"object.all_proxy_validations" + scope.oneSide("server");
			if (db.has(all_proxy_validations_key)) {
				XmlParagraph p4;
				p4 << XmlText("More details about proxy validation "
					"effectiveness are available ")
					<< db.ptr(all_proxy_validations_key, XmlText("elsewhere"))
					<< XmlText(".");
				descr << p4;
			}

			blob << descr;
		}

	} else {
		XmlParagraph para;
		para << XmlText(theOneSideWarn)
			<< XmlText("No cheap proxy validation measurements"
				" can be derived from one side logs.");
		blob << para;
	}

	db << blob;
}

void TestInfo::cmplCheapProxyValidationTable(BlobDb &db, XmlTag &parent, const Scope &scope) {
	Assert(twoSided());

	static const String tlTitle = "cheap proxy validation table";
	ReportBlob blob(BlobDb::Key("cheap_proxy_validation.ratio.table", scope), tlTitle);
	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Cheap proxy validation");

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
		tr << algnLeft << XmlTableHeading("measured");

		XmlTableCell dhr;
		dhr << algnRight
			<< db.quote("cheap_proxy_validation.ratio.obj" + scope);
		tr << dhr;

		XmlTableCell bhr;
		bhr << algnRight
			<< db.quote("cheap_proxy_validation.ratio.byte" + scope);
		tr << bhr;

		table << tr;
	}

	blob << table;
	db << blob;
	parent << blob;
}

void TestInfo::cmplByteLatencyVars(BlobDb &db, const Scope &scope) {
	if (twoSided()) {
		const String sfx = BlobDb::KeySuffix(scope);
		const AggrStat last_req_byte_written(lastReqByteWritten(scope));
		const AggrStat last_req_byte_read(lastReqByteRead(scope));
		const AggrStat first_resp_byte_written(firstRespByteWritten(scope));
		const AggrStat first_resp_byte_read(firstRespByteRead(scope));
		addMeasBlob(db, "latency.request.last_byte_written.mean" + sfx, last_req_byte_written.mean(), "msec", "mean last request byte written latency");
		addMeasBlob(db, "latency.request.last_byte_written.min" + sfx, last_req_byte_written.min(), "msec", "min last request byte written latency");
		addMeasBlob(db, "latency.request.last_byte_written.max" + sfx, last_req_byte_written.max(), "msec", "max last request byte written latency");
		addMeasBlob(db, "latency.request.last_byte_read.mean" + sfx, last_req_byte_read.mean(), "msec", "mean last request byte read latency");
		addMeasBlob(db, "latency.request.last_byte_read.min" + sfx, last_req_byte_read.min(), "msec", "min last request byte read latency");
		addMeasBlob(db, "latency.request.last_byte_read.max" + sfx, last_req_byte_read.max(), "msec", "max last request byte read latency");
		addMeasBlob(db, "latency.response.first_byte_written.mean" + sfx, first_resp_byte_written.mean(), "msec", "mean first response byte written latency");
		addMeasBlob(db, "latency.response.first_byte_written.min" + sfx, first_resp_byte_written.min(), "msec", "min first response byte written latency");
		addMeasBlob(db, "latency.response.first_byte_written.max" + sfx, first_resp_byte_written.max(), "msec", "max first response byte written latency");
		addMeasBlob(db, "latency.response.first_byte_read.mean" + sfx, first_resp_byte_read.mean(), "msec", "mean first response byte read latency");
		addMeasBlob(db, "latency.response.first_byte_read.min" + sfx, first_resp_byte_read.min(), "msec", "min first response byte read latency");
		addMeasBlob(db, "latency.response.first_byte_read.max" + sfx, first_resp_byte_read.max(), "msec", "max first response byte read latency");
	} else {
		// XXX: put err pointer to the theOneSideWarn-based description
		Should(false);
	}
}

void TestInfo::cmplByteLatency(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "latency";
	ReportBlob blob(BlobDb::Key("latency", scope), tlTitle);
	blob << XmlAttr("vprimitive", "Byte Latency");

	if (twoSided()) {
		cmplByteLatencyTable(db, blob, scope);
		cmplByteLatencyHist(db, blob, scope);

		{
			XmlTag descr("description");

			XmlTextTag<XmlParagraph> p1;
			p1.buf() << "The 'first response byte' latency is the "
				"time it took Polygraph to read (or write) the "
				"first response byte. The timer starts when the "
				"transaction starts. The timer stops when the "
				"server writes the first response byte to the "
				"TCP socket or the client reads the first "
				"response byte from the socket.";
			descr << p1;

			XmlTextTag<XmlParagraph> p2;
			p2.buf() << "Similarly, the 'last request byte' latency "
				"is the time it took Polygraph to read (or "
				"write) the last request byte. The timer starts "
				"when the transaction starts. The timer stop "
				"when the client writes the last request byte "
				"or the server reads the last request byte.";
			descr << p2;

			XmlTextTag<XmlParagraph> p3;
			p3.buf() << "Usually, more than one byte is read or "
				"written in one I/O operation, but a "
				"single-byte I/O is sufficient to stop these "
				"latency timers. Only HTTP-level bytes can stop "
				"the timers. Low-level content exchanged during "
				"TCP or SSL handshakes and negotiations has no "
				"effect. These stats are collected for basic "
				"transactions only.";
			descr << p3;

			blob << descr;
		}
	} else {
		XmlParagraph para;
		para << XmlText(theOneSideWarn)
			<< XmlText("No latency measurements"
				" can be derived from one side logs.");
		blob << para;
	}

	db << blob;
}

void TestInfo::cmplByteLatencyTable(BlobDb &db, XmlTag &parent, const Scope &scope) {
	Assert(twoSided());

	static const String tlTitle = "byte latency table";
	ReportBlob blob(BlobDb::Key("latency.table", scope), tlTitle);
	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);

	{
		XmlTableRec tr;
		tr << XmlTableHeading("Byte Latency", 1, 2);
		tr << XmlTableHeading("Written (msec)", 3, 1);
		tr << XmlTableHeading("Read (msec)", 3, 1);
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

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("Last request byte");

		XmlTableCell wmin;
		wmin << algnRight
			<< db.quote("latency.request.last_byte_written.min" + scope);
		tr << wmin;

		XmlTableCell wmean;
		wmean << algnRight
			<< db.quote("latency.request.last_byte_written.mean" + scope);
		tr << wmean;

		XmlTableCell wmax;
		wmax << algnRight
			<< db.quote("latency.request.last_byte_written.max" + scope);
		tr << wmax;

		XmlTableCell rmin;
		rmin << algnRight
			<< db.quote("latency.request.last_byte_read.min" + scope);
		tr << rmin;

		XmlTableCell rmean;
		rmean << algnRight
			<< db.quote("latency.request.last_byte_read.mean" + scope);
		tr << rmean;

		XmlTableCell rmax;
		rmax << algnRight
			<< db.quote("latency.request.last_byte_read.max" + scope);
		tr << rmax;

		table << tr;
	}

	{
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("First response byte");

		XmlTableCell wmin;
		wmin << algnRight
			<< db.quote("latency.response.first_byte_written.min" + scope);
		tr << wmin;

		XmlTableCell wmean;
		wmean << algnRight
			<< db.quote("latency.response.first_byte_written.mean" + scope);
		tr << wmean;

		XmlTableCell wmax;
		wmax << algnRight
			<< db.quote("latency.response.first_byte_written.max" + scope);
		tr << wmax;

		XmlTableCell rmin;
		rmin << algnRight
			<< db.quote("latency.response.first_byte_read.min" + scope);
		tr << rmin;

		XmlTableCell rmean;
		rmean << algnRight
			<< db.quote("latency.response.first_byte_read.mean" + scope);
		tr << rmean;

		XmlTableCell rmax;
		rmax << algnRight
			<< db.quote("latency.response.first_byte_read.max" + scope);
		tr << rmax;

		table << tr;
	}

	blob << table;
	db << blob;
	parent << blob;
}

void TestInfo::cmplByteLatencyHist(BlobDb &db, XmlTag &parent, const Scope &scope) {
	Assert(twoSided());

	static const String tlTitle = "byte latency histogram";
	ReportBlob blob(BlobDb::Key("latency.histogram", scope), tlTitle);

	{
		TimeHistStex stex1("latency.request.last_byte_written.stex" + scope, "last byte written", &StatPhaseRec::theLastReqByteWritten);
		TimeHistStex stex2("latency.response.first_byte_read.stex" + scope, "first byte read", &StatPhaseRec::theFirstRespByteRead);
		HistogramFigure fig;
		fig.configure("latency.histogram.client_figure" + scope, "Client Byte Latency Histogram");
		fig.stats(&stex1, &cltSide().phase(scope));
		fig.compareWith(&stex2);
		const String &figKey = fig.plot(db).key();
		blob << db.include(figKey);
	}

	{
		TimeHistStex stex1("latency.request.last_byte_read.stex" + scope, "last byte read", &StatPhaseRec::theLastReqByteRead);
		TimeHistStex stex2("latency.response.first_byte_written.stex" + scope, "first byte written", &StatPhaseRec::theFirstRespByteWritten);
		HistogramFigure fig;
		fig.configure("latency.histogram.server_figure" + scope, "Server Byte Latency Histogram");
		fig.stats(&stex1, &srvSide().phase(scope));
		fig.compareWith(&stex2);
		const String &figKey = fig.plot(db).key();
		blob << db.include(figKey);
	}

	parent << blob;
}

void TestInfo::cmplBaseStats(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "baseline stats";
	ReportBlob blob(BlobDb::Key("baseline", scope), tlTitle);

	blob << db.quote(BlobDb::Key("load", scope));
	blob << db.quote(BlobDb::Key("hit.ratio", scope));

	db << blob;
}

void TestInfo::cmplTraffic(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "test traffic stats";
	ReportBlob blob("traffic" + scope, tlTitle);

	XmlTag title("title");
	title << XmlText("Traffic rates, counts, and volumes");
	blob << title;

	blob << XmlTextTag<XmlParagraph>("This information is based on the client-side measurements.");

	blob << db.quote(BlobDb::Key("load", scope.oneSide("client")));
	blob << db.quote(BlobDb::Key("ssl.load", scope.oneSide("client")));
	blob << db.quote(BlobDb::Key("ftp.load", scope.oneSide("client")));
	blob << db.quote(BlobDb::Key("reply_stream.table", scope.oneSide("client")));

	db << blob;
}

void TestInfo::cmplRptm(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "test response time stats";
	ReportBlob blob("rptm" + scope, tlTitle);

	XmlTag title("title");
	title << XmlText("Response times");
	blob << title;

	blob << XmlTextTag<XmlParagraph>("This information is based on the client-side measurements.");

	blob << db.quote(BlobDb::Key("rptm.trace", scope.oneSide("client")));
	blob << db.quote("reply_object.table" + scope.oneSide("client"));

	blob << db.quote(BlobDb::Key("latency", scope));

	db << blob;
}

void TestInfo::cmplSavings(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "cache effectiveness";
	ReportBlob blob("savings" + scope, tlTitle);

	XmlTag title("title");
	title << XmlText("Savings");
	blob << title;

	blob << db.quote("hit.ratio" + scope);

	blob << db.quote("hit.ratio" + scope.oneSide("client"));

	blob << db.quote("cheap_proxy_validation.ratio" + scope);

	db << blob;
}

void TestInfo::cmplLevels(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "test transaction concurrency and population levels";
	ReportBlob blob("levels" + scope, tlTitle);

	XmlTag title("title");
	title << XmlText("Concurrency levels and robot population");
	blob << title;

	blob << XmlTextTag<XmlParagraph>("This information is based on the client-side measurements.");
	const InfoScope cltScope = scope.oneSide("client");

	{
		XmlSection s("concurrent HTTP/TCP connections");
		//s << db.quote("conn.level.fig" + cltScope);
		s << db.quote("conn.level.table" + cltScope);
		blob << s;
	}
		
	{
		XmlSection s("population level");
		//s << db.quote("populus.level.fig" + cltScope);
		s << db.quote("populus.level.table" + cltScope);
		blob << s;
	}
		
	{
		XmlSection s("concurrent HTTP transactions");
		//s << db.quote("xact.level.fig" + cltScope);
		s << db.quote("xact.level.table" + cltScope);
		blob << s;
	}
		
	db << blob;
}

void TestInfo::cmplAuthentication(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "test authentication stats";
	ReportBlob blob("authentication" + scope, tlTitle);

	XmlTag title("title");
	title << XmlText("Authentication");
	blob << title;

	blob << XmlTextTag<XmlParagraph>("This information is based on the client-side measurements.");
	const InfoScope cltScope = scope.oneSide("client");

	{
		XmlTag compoundReplyStreamTitle("h3");
		compoundReplyStreamTitle << XmlText("Compound reply traffic stream table");
		blob << compoundReplyStreamTitle;
		blob << db.quote(BlobDb::Key("compound.reply_stream.table", cltScope));
	}

	{
		XmlTag compoundRequestStreamTitle("h3");
		compoundRequestStreamTitle << XmlText("Compound request traffic stream table");
		blob << compoundRequestStreamTitle;
		blob << db.quote(BlobDb::Key("compound.request_stream.table", cltScope));
	}

	{
		XmlTag compoundReplyObjectTitle("h3");
		compoundReplyObjectTitle << XmlText("Compound reply object kind table");
		blob << compoundReplyObjectTitle;
		blob << db.quote(BlobDb::Key("compound.reply_object.table", cltScope));
	}

	{
		XmlTag compoundRequestObjectTitle("h3");
		compoundRequestObjectTitle << XmlText("Compound request object kind table");
		blob << compoundRequestObjectTitle;
		blob << db.quote(BlobDb::Key("compound.request_object.table", cltScope));
	}

	{
		XmlTag authStreamTitle("h3");
		authStreamTitle << XmlText("Authentication traffic stream table");
		blob << authStreamTitle;
		blob << db.quote(BlobDb::Key("auth.stream.table", cltScope));
	}

	{
		XmlTag authObjectTitle("h3");
		authObjectTitle << XmlText("Authentication object kind table");
		blob << authObjectTitle;
		blob << db.quote(BlobDb::Key("auth.object.table", cltScope));
	}

	db << blob;
}

void TestInfo::cmplErrors(BlobDb &db, const Scope &scope) {
	static const String tlTitle = "test errors";
	ReportBlob blob("errors" + scope, tlTitle);

	XmlTag title("title");
	title << XmlText("Errors");
	blob << title;

	{
		XmlSection s("client-side errors");
		s << db.include("errors.table" + scope.oneSide("client"));
		blob << s;
	}
		
	{
		XmlSection s("server-side errors");
		s << db.include("errors.table" + scope.oneSide("server"));
		blob << s;
	}
		
	db << blob;
}

void TestInfo::cmplNotes(BlobDb &db) {
	static const String tlTitle = "report notes";
	ReportBlob blob("report_notes", tlTitle);

	XmlSearchRes res;
	if (db.blobs().selByAttrName("report_note", res)) {
		XmlTag list("ol");
		for (int i = 0; i < res.count(); ++i) {
			Assert(res[i]->attrs()->value("report_note_number") == AnyToString(i + 1));
			list << db.include(res[i]->attrs()->value("key"));
		}
		blob << list;
	}

	db << blob;
}

void TestInfo::cmplSynonyms(BlobDb &db, const Scope &scope) {
	addLink(db, 
		BlobDb::Key("load", scope),
		BlobDb::Key("load", scope.oneSide("client")));
	addLink(db, 
		BlobDb::Key("load.table", scope),
		BlobDb::Key("load.table", scope.oneSide("client")));
	addLink(db,
		BlobDb::Key("reply_stream.table", scope),
		BlobDb::Key("reply_stream.table", scope.oneSide("client")));
}

void TestInfo::compileStats(BlobDb &db) {

	if (!theExecScope)
		guessExecScope();

	if (cltSideExists())
		cltSide().compileStats(db);
	else
		SideInfo::CompileEmptyStats(db, execScope().oneSide("client"));
	if (srvSideExists())
		srvSide().compileStats(db);
	else
		SideInfo::CompileEmptyStats(db, execScope().oneSide("server"));

	cmplExecSumVars(db);
	cmplExecSum(db);
	cmplWorkload(db);

	// build theScopes array
	theScopes.append(new Scope(execScope()));
	if (twoSided()) {
		Scope *allScope = new Scope;
		allScope->name("all phases");
		theScopes.append(allScope);
		for (int i = 0; i < cltSide().phaseCount(); ++i) {
			const String &pname = cltSide().phase(i).name();
			// include common phases only
			if (!srvSide().scope().hasPhase(pname))
				continue;
			theScopes.append(new Scope(theExecScope.onePhase(pname)));
			theScopes.last()->name(pname);
			allScope->add(*theScopes.last());
		}
	}

	for (int s = 0; s < theScopes.count(); ++s) {
		const Scope &scope = *theScopes[s];
		cmplSynonyms(db, scope);
		cmplHitRatioVars(db, scope);
		cmplHitRatio(db, scope);
		cmplCheapProxyValidationVars(db, scope);
		cmplCheapProxyValidation(db, scope);
		cmplByteLatencyVars(db, scope);
		cmplByteLatency(db, scope);
		cmplBaseStats(db, scope);
		cmplTraffic(db, scope);
		cmplRptm(db, scope);
		cmplSavings(db, scope);
		cmplLevels(db, scope);
		cmplAuthentication(db, scope);
		cmplErrors(db, scope);
	}

	cmplNotes(db);
}
