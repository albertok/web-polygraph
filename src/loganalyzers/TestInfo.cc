
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


TestInfo::TestInfo(const String &aLabel): theLabel(aLabel) {
	theSides.resize(lgcEnd);
	theSides[lgcCltSide] = new SideInfo(lgcCltSide);
	theSides[lgcCltSide]->test(this);
	theSides[lgcSrvSide] = new SideInfo(lgcSrvSide);
	theSides[lgcSrvSide]->test(this);
}

TestInfo::~TestInfo() {
	while (theSides.count()) {
		if (theSides.last())
			theSides.last()->test(0);
		delete theSides.pop();
	}

	while (theScopes.count()) delete theScopes.pop();
}

void TestInfo::compileExecScope(BlobDb &db) {
	Assert(!theExecScope);
	theExecScope.name("baseline");
	if (cltSideExists()) {
		theExecScope.addSide("client");
		cltSide().compileExecScope(0);
	}
	if (srvSideExists()) {
		theExecScope.addSide("server");
		srvSide().compileExecScope(cltSideExists() ? &cltSide().execScope() : 0);
	}
	// now either both sides have identical baseline phases or
	// at most one side has a baseline.

	if (!twoSided()) {
		theExecScope.reason = "No test-wide baseline stats are available "
			"because they require both client- and server-side logs but only "+
			aSide().name() + " log(s) were provided.";
		return; // one side, no phases
	}

	if (!cltSide().execScope()) {
		theExecScope = cltSide().execScope();
		theExecScope.reason = "No test-wide baseline stats are available "
			"because no client-side baseline stats are available.";
		theExecScope.addSide("server");
	} else
	if (!srvSide().execScope()) {
		theExecScope = srvSide().execScope();
		theExecScope.reason = "No test-wide baseline stats are available "
			"because no server-side baseline stats are available.";
		theExecScope.addSide("client");
	} else {
		const String cltPhases = PointersToString(cltSide().execScope().phases(), ", ");
		const String srvPhases = PointersToString(srvSide().execScope().phases(), ", ");
		Assert(cltPhases == srvPhases);
		theExecScope = cltSide().execScope();
		theExecScope.addSide("server");
		theExecScope.reason = "Test-wide baseline stats are based on the "
			"following phase(s): " + cltPhases + ".";
	}

	// theExecScope phases are empty if either client or server phases are
}

const String &TestInfo::label() const {
	return theLabel;
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

bool TestInfo::hasScope(const InfoScope &scope) const {
	bool found = false;
	for (int i = 0; !found && i < theScopes.count(); ++i)
		found = *theScopes[i] == scope;
	return found;
}

int TestInfo::scopes(InfoScopes &res) const {
	if (!twoSided())
		return aSide().scopes(res);

	for (int i = 0; i < theScopes.count(); ++i)
		res.add(*theScopes[i]);

	return res.count();
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
	checkCommonStartTime();
}

Counter TestInfo::repCount(const Scope &scope) const {
	return cltSideExists() ? cltSide().repCount(scope) : -1;
}

Counter TestInfo::hitCount(const Scope &scope) const {
	return twoSided() ? cltSide().repCount(scope) - srvSide().repCount(scope) : -1;
}

Counter TestInfo::uselessProxyValidationCount(const Scope &scope) const {
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
		static const String tlTitle = "baseline selection argumentation";
		ReportBlob blob("baseline.reason.test", tlTitle);
		blob << XmlText(theExecScope.reason);
		db << blob;
	}

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
			p << db.ptr(BlobDb::Key("benchmark.version", cltSide().execScope()), XmlText("client-"));
			p << XmlText(" and ");
			p << db.ptr(BlobDb::Key("benchmark.version", srvSide().execScope()), XmlText("server-side"));
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
	cmplExecSumVars(db);
	cmplExecSumTable(db);
}

void TestInfo::cmplExecSumTable(BlobDb &db) {
	const Scope *cltScope = cltSideExists() && cltSideExists()->execScope() ?
		&cltSideExists()->execScope() : 0;

	static const String tlTitle = "executive summary table";
	ReportBlob blob("summary.exec.table", tlTitle);
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

	if (cltScope) {
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("throughput:");

		XmlTableCell cell;
		cell << db.quote("rep.rate" + *cltScope);
		cell << XmlText(" or ");
		cell << db.quote("rep.bwidth" + *cltScope);
		tr << cell;

		table << tr;
	}

	if (cltScope) {
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("response time:");

		XmlTableCell cell;
		//cell << db.quote("object.hits.rptm.mean" + *cltScope);
		//cell << XmlText(" hit, ");
		cell << db.quote("rep.rptm.mean" + *cltScope);
		cell << XmlText(" mean");
		//cell << db.quote("object.misses.rptm.mean" + *cltScope);
		//cell << XmlText(" miss");
		tr << cell;

		table << tr;
	}

	if (theExecScope) {
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

	if (cltScope) {
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("unique URLs:");

		XmlTableCell cell;
		cell << db.quote("url.unique.count" + *cltScope);
		cell << XmlText(" (");
		cell << db.quote("url.recurrence.ratio" + *cltScope);
		cell << XmlText(" recurrence)");
		tr << cell;

		table << tr;
	}

	if (cltScope) {
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("errors:");

		XmlTableCell cell;
		cell << db.quote("xact.error.ratio" + *cltScope);
		cell << XmlText(" (");
		cell << db.quote("xact.error.count" + *cltScope);
		cell << XmlText(" out of ");
		cell << db.quote("xact.count" + *cltScope);
		cell << XmlText(")");
		tr << cell;

		table << tr;
	}

	if (cltScope) {
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("duration:");

		XmlTableCell cell;
		cell << db.include("duration" + *cltScope);
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

	/* XXX: check that workload page is available; it may be even theExecScope is empty */ {
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("workload:");

		XmlTableCell cell;
		cell << db.ptr("page.workload", XmlText("available"));
		tr << cell;

		table << tr;
	}

	/* XXX: check that a single version is available or report all versions */ {
		XmlTableRec tr;
		tr << algnLeft << XmlTableHeading("Polygraph version:");

		XmlTableCell cell;
		cell << db.include("benchmark.version" + theExecScope);
		tr << cell;

		table << tr;
	}

	/* XXX: remove theExecScope */ {
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

void TestInfo::cmplWorkload(BlobDb &db) {
	static const String tlTitle = "test workload";
	ReportBlob blob("workload.test", tlTitle);

	String oneCfg;
	if (!cltSideExists() && srvSideExists())
		oneCfg = srvSide().pglCfg();
	else
	if (cltSideExists() && !srvSideExists())
		oneCfg = cltSide().pglCfg();
	else
	if (cltSide().pglCfg() == srvSide().pglCfg())
		oneCfg = cltSide().pglCfg();
	else {
		XmlParagraph p;
		p << XmlText("Cannot show a single test workload because ");
		p << db.ptr("workload.code.client", XmlText("client-"));
		p << XmlText(" and ");
		p << db.ptr("workload.code.server", XmlText("server-side"));
		p << XmlText(" PGL configurations differ.");
		p << db.reportNote("workload.test", db.ptr("page.workload", XmlText("client- and server-side PGL configurations differ")));
		blob << p;
	}

	if (oneCfg)
		cmplWorkloadBlob(blob, "", "workload.code.test", oneCfg);
	else {
		cmplWorkloadBlob(blob, "client-side", "workload.code.client", cltSide().pglCfg());
		cmplWorkloadBlob(blob, "server-side", "workload.code.server", srvSide().pglCfg());
	}

	// TODO: Add English interpretation.

	db << blob;
}

void TestInfo::cmplWorkloadBlob(ReportBlob &blob, const String &pfx, const String &key, const String &pglCfg) {
	{
		const String tlPglTitle(pfx + "PGL code");

		XmlSection sect(tlPglTitle);

		ReportBlob code(key, tlPglTitle);
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

	const String hitRatioKey = "hit.ratio" + scope.oneSide("client");

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

			if (db.has(hitRatioKey)) {
			XmlParagraph p3;
			p3 << XmlText("A less accurate way to measure hit ratio is to "
				"detect hits on the client-side using custom HTTP headers. "
				"A hit ratio table based on client-side tricks is available ");
			p3 << db.ptr(hitRatioKey, XmlText("elsewhere"));
			p3 << XmlText(".");
			descr << p3;
			}

			blob << descr;
		}

	} else {
		XmlParagraph para;
		para << XmlText(theOneSideWarn);
		if (db.has(hitRatioKey)) {
			para << XmlText("See ");
			para << db.ptr(hitRatioKey, XmlText("client-side"));
			para << XmlText(" information for hit ratio estimations (if any).");
		} else {
			para << XmlText("No hit ratio measurements"
				" can be derived from server-side logs.");
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

void TestInfo::cmplTraffic(BlobDb &db) {
	if (!aSide().execScope())
		return;

	static const String tlTitle = "test traffic stats";
	ReportBlob blob("traffic", tlTitle);

	XmlTag title("title");
	title << XmlText("Traffic rates, counts, and volumes");
	blob << title;

	blob << XmlTextTag<XmlParagraph>("This information is based on the " +
		aSide().name() + "-side baseline measurements.");
	
	const InfoScope &scope = aSide().execScope();
	blob << db.quote(BlobDb::Key("load", scope));
	blob << db.quote(BlobDb::Key("ssl.load", scope));
	blob << db.quote(BlobDb::Key("ftp.load", scope));
	blob << db.quote(BlobDb::Key("reply_stream.table", scope));

	db << blob;
}

void TestInfo::cmplRptm(BlobDb &db) {
	if (!aSide().execScope())
		return;

	static const String tlTitle = "test response time stats";
	ReportBlob blob("rptm", tlTitle);

	XmlTag title("title");
	title << XmlText("Response times");
	blob << title;

	XmlParagraph p;
	p << XmlText("This information is based on the " +
		aSide().name() + "-side baseline measurements");
	if (theExecScope)
		p << XmlText(", except for latency figures that use both test sides");
	p << XmlText(".");
	blob << p;
	
	const InfoScope &scope = aSide().execScope();
	blob << db.quote(BlobDb::Key("rptm.trace", scope));
	blob << db.quote("reply_object.table" + scope);

	if (theExecScope)
		blob << db.quote(BlobDb::Key("latency", theExecScope));

	db << blob;
}

void TestInfo::cmplSavings(BlobDb &db) {
	if (!cltSideExists() || !cltSideExists()->execScope())
		return;
	const InfoScope &cltScope = cltSideExists()->execScope();

	static const String tlTitle = "cache effectiveness";
	ReportBlob blob("savings", tlTitle);

	XmlTag title("title");
	title << XmlText("Savings");
	blob << title;

	if (theExecScope)
		blob << db.quote("hit.ratio" + theExecScope);

	blob << db.quote("hit.ratio" + cltScope);

	if (theExecScope)
		blob << db.quote("cheap_proxy_validation.ratio" + theExecScope);

	db << blob;
}

void TestInfo::cmplLevels(BlobDb &db) {
	if (!aSide().execScope())
		return;

	static const String tlTitle = "test transaction concurrency and population levels";
	ReportBlob blob("levels", tlTitle);

	XmlTag title("title");
	title << XmlText("Concurrency levels and robot population");
	blob << title;

	blob << XmlTextTag<XmlParagraph>("This information is based on the " +
		aSide().name() + "-side baseline measurements.");

	const InfoScope &scope = aSide().execScope();

	{
		XmlSection s("concurrent HTTP/TCP connections");
		//s << db.quote("conn.level.fig" + scope);
		s << db.quote("conn.level.table" + scope);
		blob << s;
	}
		
	{
		XmlSection s("population level");
		//s << db.quote("populus.level.fig" + scope);
		s << db.quote("populus.level.table" + scope);
		blob << s;
	}
		
	{
		XmlSection s("concurrent HTTP transactions");
		//s << db.quote("xact.level.fig" + scope);
		s << db.quote("xact.level.table" + scope);
		blob << s;
	}
		
	db << blob;
}

void TestInfo::cmplAuthentication(BlobDb &db) {
	if (!cltSideExists() || !cltSideExists()->execScope())
		return;
	const InfoScope &cltScope = cltSideExists()->execScope();

	static const String tlTitle = "test authentication stats";
	ReportBlob blob("authentication", tlTitle);

	XmlTag title("title");
	title << XmlText("Authentication");
	blob << title;

	blob << XmlTextTag<XmlParagraph>("This information is based on the client-side baseline measurements.");

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

void TestInfo::cmplErrors(BlobDb &db) {
	static const String tlTitle = "test errors";
	ReportBlob blob("errors", tlTitle);

	XmlTag title("title");
	title << XmlText("Errors");
	blob << title;

	if (cltSideExists()) {
		XmlSection s("client-side errors");
		if (cltSideExists()->execScope())
			s << db.include("errors.table" + cltSideExists()->execScope());
		else
			s << db.quote("baseline.reason.client");
		blob << s;
	}

	if (srvSideExists()) {
		XmlSection s("server-side errors");
		if (srvSideExists()->execScope())
			s << db.include("errors.table" + srvSideExists()->execScope());
		else
			s << db.quote("baseline.reason.server");
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

void TestInfo::compileStats(BlobDb &db) {
	compileExecScope(db);

	// build theScopes array
	// include empty scopes to improve "everything.html" tables
	theScopes.append(new Scope(execScope()));
	if (twoSided()) {
		Scope *allScope = new Scope;
		allScope->name("all phases"); // may be renamed later
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
		if (allScope->phases().count() != cltSide().phaseCount() ||
			allScope->phases().count() != srvSide().phaseCount())
			allScope->rename("common phases");
	}

	if (cltSideExists())
		cltSide().compileStats(db);
	if (srvSideExists())
		srvSide().compileStats(db);

	cmplExecSum(db);
	cmplWorkload(db);
	cmplTraffic(db);
	cmplRptm(db);
	cmplSavings(db);
	cmplLevels(db);
	cmplAuthentication(db);
	cmplErrors(db);

	for (int s = 0; s < theScopes.count(); ++s) {
		const Scope &scope = *theScopes[s];
		if (!scope)
			continue;
		cmplHitRatioVars(db, scope);
		cmplHitRatio(db, scope);
		cmplCheapProxyValidationVars(db, scope);
		cmplCheapProxyValidation(db, scope);
		cmplByteLatencyVars(db, scope);
		cmplByteLatency(db, scope);
	}

	cmplNotes(db);
}
