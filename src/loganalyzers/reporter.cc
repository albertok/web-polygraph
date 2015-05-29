
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include "xstd/h/iostream.h"
#include <fstream>
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"
#include "xstd/h/new.h"
#include "xstd/Map.h"

#include "base/StatPhaseRec.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "base/CmdLine.h"
#include "base/opts.h"
#include "base/AnyToString.h"
#include "xml/XmlAttr.h"
#include "xml/XmlNodes.h"
#include "xml/XmlDoc.h"
#include "xml/XmlText.h"
#include "xml/XmlParagraph.h"
#include "xml/XmlTable.h"
#include "xml/XmlSection.h"
#include "logextractors/LogIter.h"
#include "loganalyzers/InfoScopes.h"
#include "loganalyzers/PhaseTrace.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/ProcInfo.h"
#include "loganalyzers/SideInfo.h"
#include "loganalyzers/TestInfo.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/ReportFigure.h"
#include "loganalyzers/RepToHtmlFile.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/RepOpts.h"

static TestInfo *TheTest = 0;

typedef Map<int> PhaseNames; // raw name -> count
static PhaseNames ThePhaseNames;

/* local routines */

String uniquePhaseName(const String &rawName) {
	if (int *count = ThePhaseNames.valp(rawName)) {
		Should(*count > 0);
		++*count;
		return uniquePhaseName(rawName + "-n" + AnyToString(*count));
	}

	ThePhaseNames.add(rawName, 1);
	return rawName;
}

void checkPhaseNames(const String &fname) {
	bool errors = false;
	for (int i = 0; i < ThePhaseNames.count(); ++i) {
		const int count = ThePhaseNames.valAt(i);
		if (count > 1) {
			clog << fname << ": warning: found " << count <<
				" phases named '" << ThePhaseNames.keyAt(i) << "'" << endl;
			errors = true;
		}
	}

	if (errors)
		clog << fname << ": warning: appending unique suffixes to " <<
			"phase names to avoid name clashes" << endl;
}

void resetPhaseNames() {
	ThePhaseNames.reset();
}

static
ProcInfo *scanLog1(LogIter &li) {
	ProcInfo *proc = new ProcInfo(li.log().fileName());
	proc->startTime(li ? li.log().progress().time() : Time());

	String phaseName;
	resetPhaseNames();
	bool needComments = true;

	while (li) {
		switch (li->theTag) {
			case lgComment: {
				// XXX: add extraction of shutdown reason
				if (needComments) {
					String comment;
					li.log() >> comment;
					if (!comment.str("Configuration:"))
						break;
					const String kword = "version:";
					if (const char *beg = comment.str(kword.cstr())) {
						beg += kword.len();
						while (*beg && isspace(*beg)) ++beg;
						const char *end = beg;
						while (*end && !isspace(*end)) ++end;
						if (Should(end > beg)) {
							proc->benchmarkVersion(
								comment(beg-comment.cstr(), end-comment.cstr()));
						}
					}
					needComments = false;
				}
				break;
			}
			
			case lgContTypeKinds: {
				// should be called only once per log
				ContTypeStat::Load(li.log());
				break;
			}

			case lgPglCfg: {
				String cfg;
				li.log() >> cfg;
				proc->pglCfg(cfg);
				break;
			}
			
			case lgStatPhaseBeg: {
				String rawName;
				li.log() >> rawName;
				phaseName = uniquePhaseName(rawName);
				break;
			}

			case lgStatPhaseEnd: {
				phaseName = 0;
				break;
			}

			case lgStatCycleRec: {
				const int cat = li->theCat;
				if (!Should(cat == lgcCltSide || cat == lgcSrvSide))
					break;
				if (proc->logCat() == lgcEnd)
					proc->logCat(cat);

				if (proc->logCat() == cat) {
					StatIntvlRec r;
					r.load(li.log());
					if (Should(r.sane()))
						proc->noteIntvl(r, phaseName);
				}
				break;
			}

			case lgStatPhaseRec: {
				const int cat = li->theCat;
				if (proc->logCat() == cat) {
					StatPhaseRec r;
					r.load(li.log());
					if (Should(r.sane()))
						proc->addPhase(r);
				}
				break;
			}
		}
		++li;
	}

	if (proc->logCat() == lgcEnd) {
		cerr << li.log().fileName() 
			<< ": error: cannot determine log 'side', skipping" << endl;
		delete proc;
		return 0;
	}

	proc->noteEndOfLog();

	checkPhaseNames(li.log().fileName());
	resetPhaseNames();
	return proc;
}

static
void scanLog2(LogIter &li, ProcInfo *proc) {
	PhaseTrace *trace = 0;

	while (li) {
		switch (li->theTag) {
			case lgStatPhaseBeg: {
				String rawName, phaseName;
				li.log() >> rawName;
				phaseName = uniquePhaseName(rawName);
				if (proc->hasPhase(phaseName))
					trace = proc->tracePhase(phaseName);
				break;
			}

			case lgStatPhaseEnd: {
				trace = 0;
				break;
			}

			case lgStatCycleRec: {
				const int cat = li->theCat;
				if (Should(trace) && proc->logCat() == cat) {
					StatIntvlRec r;
					r.load(li.log());
					if (Should(r.sane()))
						trace->addIntvl(li.log().progress().time(), r);
				}
				break;
			}
		}
		++li;
	}
}

static
void scanAll() {
	for (int i = 0; i < TheRepOpts.theFiles.count(); ++i) {
		const String &fname = *TheRepOpts.theFiles[i];
		ILog log;
		if (fname == "-")
			log.stream("stdin", &cin);
		else
			log.stream(fname, (istream*)new ifstream(fname.cstr(), ios::binary|ios::in));

		clog << "scanning " << fname << endl;

		LogIter li(&log);
		if (ProcInfo *proc = scanLog1(li)) {
			ILog log2;
			log.stream()->clear();
			log.stream()->seekg(0, ios::beg);
			log2.stream(log.fileName(), log.stream());
			LogIter li2(&log2);
			scanLog2(li2, proc);

			TheTest->side(proc->logCat()).add(proc);
		}
	}
}

static
void checkConsistency() {
	clog << "checking consistency" << endl;
	TheTest->checkConsistency();
}

static
void compileStats(BlobDb &db) {
	clog << "compiling statistics" << endl;
	SideInfo::Configure();
	TheTest->compileStats(db);
}

static
String htmlFileName(const String &baseName) {
	return TheRepOpts.theRepDir + '/' + baseName + ".html";
}

static
XmlTag &addTitle(BlobDb &db, XmlTag &ctx, const String &text) {
    const XmlNode &prefix = db.ptr("summary.front" + TheTest->execScope(),
		XmlText(TheTest->label()));

	XmlText suffix;
	suffix.buf() << ": " << text;

	XmlTag *title = new XmlTag("title");
	*title << prefix << suffix;
	ctx.addChild(title);
	return *title;
}

#if 0
static
void alignRight(XmlTag &parent, const XmlNodes &subject) {
	XmlTable table;
	table << XmlAttr::Int("border", 0) << XmlAttr("align", "right")
		<< XmlAttr::Int("cellspacing", 0) << XmlAttr::Int("cellpadding", 0);

	XmlTableRec tr;
	XmlTableCell cell;
	cell << subject;
	tr << cell;
	table << tr;

	parent << table;
}
#endif


static
void twoColumn(XmlTag &parent, const XmlNodes &lhs, const XmlNodes &rhs) {
	XmlTable table;
	table << XmlAttr::Int("border", 0)
		<< XmlAttr::Int("cellspacing", 5) << XmlAttr::Int("cellpadding", 0)
		<< XmlAttr("width", "100%");

	XmlTableRec tr;
	XmlTableCell cellLeft;
	cellLeft << XmlAttr("valign", "top") << lhs;
	XmlTableCell cellRight;
	cellRight << XmlAttr("valign", "top") << rhs;
	tr << cellLeft << cellRight;
	table << tr;

	parent << table;
}

static
const ReportBlob *buildFrontPage(BlobDb &db) {
	const InfoScope &scope = TheTest->execScope();
	ReportBlob blob("summary.front" + scope, ReportBlob::NilTitle);

	XmlTag doc("document");

	XmlTag chapter("chapter");

	XmlTag title("title");
	title << XmlText(String("Web Polygraph report: ") + TheTest->label());
	chapter << title;

	chapter << db.quote("summary.exec.table" + scope);
	
	chapter << XmlTextTag<XmlParagraph>("The following information is available.");

	XmlTag list("ul");

	list << db.ptr("summary.1page", XmlText("One-page summary"));
	//list << db.ptr("summary.2page" + scope, XmlText("Two-page summary"));
	list << db.ptr("page.traffic" + scope, XmlText("Traffic rates, counts, and volumes"));
	list << db.ptr("page.rptm" + scope, XmlText("Response times"));
	list << db.ptr("page.savings" + scope, XmlText("Hit ratios"));
	list << db.ptr("page.levels" + scope, XmlText("Concurrency levels and robot population"));
	list << db.ptr("page.auth" + scope, XmlText("Authentication"));
	list << db.ptr("page.errors" + scope, XmlText("Errors"));
	list << db.ptr("page.workload", XmlText("Workload"));
	list << db.ptr("page.everything", XmlText("Details"));
	list << db.ptr("page.notes", XmlText("Report generation notes"));

	chapter << list;
	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("index"));
	return db.add(blob);
}

static
const ReportBlob *buildOnePage(BlobDb &db) {
	ReportBlob blob("summary.1page", ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "one-page summary");

	const InfoScope &execScope = TheTest->execScope();

	XmlNodes lhs;
	lhs << db.quote("summary.exec.table" + execScope);

	XmlNodes rhs;
	rhs << db.quote("load.table" + execScope);
	//rhs << db.quote("hit.ratio.table" + execScope);

	twoColumn(chapter, lhs, rhs);

	chapter << db.quote("summary.exec.phases" + execScope);

	if (TheTest->cltSideExists()) {
		const InfoScope &scope = TheTest->cltSide().scope();
		chapter << db.quote("load.trace" + scope);
		chapter << db.quote("rptm.trace" + scope);
	}

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("one-page"));
	return db.add(blob);
}

static
const ReportBlob *buildTrafficPage(BlobDb &db, const InfoScope &scope) {
	ReportBlob blob("page.traffic" + scope, ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "traffic rates, counts, and volumes");

	chapter << db.quote("traffic" + scope);

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("traffic"));
	return db.add(blob);
}

static
const ReportBlob *buildRptmPage(BlobDb &db, const InfoScope &scope) {
	ReportBlob blob("page.rptm" + scope, ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "response times");

	chapter << db.quote("rptm" + scope);

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("rptm"));
	return db.add(blob);
}

static
const ReportBlob *buildSavingsPage(BlobDb &db, const InfoScope &scope) {
	ReportBlob blob("page.savings" + scope, ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "hit ratios");

	chapter << db.quote("savings" + scope);

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("savings"));
	return db.add(blob);
}

static
const ReportBlob *buildLevelsPage(BlobDb &db, const InfoScope &scope) {
	ReportBlob blob("page.levels" + scope, ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "concurrency levels and robot population");

	chapter << db.quote("levels" + scope);

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("levels"));
	return db.add(blob);
}

static
const ReportBlob *buildAuthPage(BlobDb &db, const InfoScope &scope) {
	ReportBlob blob("page.auth" + scope, ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "Authentication");

	chapter << db.quote("authentication" + scope);

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("authentication"));
	return db.add(blob);
}

static
const ReportBlob *buildErrorsPage(BlobDb &db, const InfoScope &scope) {
	ReportBlob blob("page.errors" + scope, ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "errors");

	chapter << db.quote("errors" + scope);

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("errors"));
	return db.add(blob);
}

static
const ReportBlob *buildWorkloadPage(BlobDb &db, const InfoScope &scope) {
	ReportBlob blob("page.workload", ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "workload");

	chapter << db.include("workload" + scope);

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("workload"));
	return db.add(blob);
}

static
const ReportBlob *buildNotesPage(BlobDb &db) {
	ReportBlob blob("page.notes", ReportBlob::NilTitle);

	XmlTag doc("document");
	XmlTag chapter("chapter");
	addTitle(db, chapter, "report generation notes");

	chapter << db.include("report_notes");

	doc << chapter;
	blob << doc;

	RepToHtmlFile::Location(db, blob, htmlFileName("notes"));
	return db.add(blob);
}

static
void addScopeRecord(BlobDb &db, const String &name, const String &label,
	const InfoScopes &scopes, const String &ctx, XmlTable &table) {

	XmlTableRec tr;
	tr << XmlTableHeading(label);

	for (int i = 0; i < scopes.count(); ++i) {
		const InfoScope &scope = *scopes[i];
		XmlTableCell cell;
		cell << XmlAttr("align", "center");
		XmlText text(scope.name());

		if (scope.image() == ctx) {
			cell << XmlAttr::Int("emphasized", true);
			cell << text;
		} else {
			XmlNode &ptr = db.ptr(name + scope, text);
			*ptr.attrs() << XmlAttr::Int("maybe_null", true);
			cell << ptr;
		}
		tr << cell;
	}

	table << tr;
}

static
void addScopeTable(BlobDb &db, const String &name,
	const InfoScopes &cltScopes, const InfoScopes &srvScopes, const InfoScopes &tstScopes,
	const String &ctx, XmlTag &tag) {

	XmlTable table;
	table << XmlAttr::Int("border", 1) << XmlAttr::Int("cellspacing", 1);
	addScopeRecord(db, name, "client side", cltScopes, ctx, table);
	addScopeRecord(db, name, "server side", srvScopes, ctx, table);
	addScopeRecord(db, name, "all sides", tstScopes, ctx, table);
	tag << table;
}

static
void buildEverything(BlobDb &db, Array<const ReportBlob *> &res) {
	const XmlNodes &blobs = db.blobs();

	InfoScopes cltScopes;
	InfoScopes srvScopes;
	InfoScopes tstScopes;
	TheTest->scopes(tstScopes);
	if (TheTest->cltSideExists())
		TheTest->cltSide().scopes(cltScopes);
	if (TheTest->srvSideExists())
		TheTest->srvSide().scopes(srvScopes);

	Map< Array<String*> *> scope2names;
	Map< Array<const char*> *> name2scopes;

	XmlSearchRes vprimitives;
	blobs.selByAttrName("vprimitive", vprimitives);

	// segregate vprimitive blobs based on their scope
	for (int p = 0; p < vprimitives.count(); ++p) {
		const String &key = vprimitives[p]->attrs()->value("key");
		if (const char *scopeImage = key.str(".scope=")) {
			const String name = key(0, scopeImage-key.cstr());

			{
				Array<String*> *names = 0;
				if (!scope2names.find(scopeImage, names)) {
					names = new Array<String*>();
					scope2names.add(scopeImage, names);
				}
				names->append(new String(name));
			}

			{
				Array<const char*> *scopes = 0;
				if (!name2scopes.find(name, scopes)) {
					scopes = new Array<const char*>();
					name2scopes.add(name, scopes);
				}
				scopes->append(scopeImage);
			}
		}
	}

	const String pfx = "page.everything";

	// for each vprimitive name, list scopes it belongs to
	{
		ReportBlob blob(pfx, ReportBlob::NilTitle);
		XmlTag doc("document");
		XmlTag chapter("chapter");
		addTitle(db, chapter, "everything (index)");

		for (int i = 0; i < name2scopes.count(); ++i) {
			const String &name = name2scopes.keyAt(i);
			const Array<const char*> *scopes = name2scopes.valAt(i);
			// get a title in hope that all titles are the same
			const ReportBlob &b = db.get(name + scopes->last());
			ReportBlob sblob(pfx + "." + name, ReportBlob::NilTitle);
			XmlSection s(b.attrs()->value("vprimitive"));
			addScopeTable(db, String("page.everything.") + name, cltScopes, srvScopes, tstScopes, 0, s);
			sblob << s;
			chapter << *db.add(sblob);
			delete scopes;
		}

		doc << chapter;
		blob << doc;
		RepToHtmlFile::Location(db, blob, htmlFileName("everything"));
		res.append(db.add(blob));		
	}

	// for each scope, create a page with corresponding vprimitives
	for (int s = 0; s < scope2names.count(); ++s) {
		const String &scopeImage = scope2names.keyAt(s);
		const Array<String*> *names = scope2names.valAt(s);

		ReportBlob blob(pfx + scopeImage, ReportBlob::NilTitle);

		XmlTag doc("document");
		XmlTag chapter("chapter");

		XmlTag &title = addTitle(db, chapter, "everything (scoped)");
		const char *ctx = scopeImage.cstr() + strlen(".scope=");
		addScopeTable(db, pfx, cltScopes, srvScopes, tstScopes, ctx, title);

		XmlTable table;
		XmlTableRec tr1, tr2;
		tr1 << XmlTableHeading("highlighted cell(s) above show current scope");
		tr2 << XmlTableHeading("links point to other scopes");
		table << tr1 << tr2;
		title << table;

		chapter << title;

		for (int i = 0; i < names->count(); ++i) {
			const String key = *names->item(i) + scopeImage;
			const ReportBlob &b = db.get(key);

			ReportBlob sblob(pfx + "." + key, ReportBlob::NilTitle);
			XmlSection s;
			s << XmlAttr("src", key);

			XmlTag stitle("title");
			stitle << db.ptr(pfx + "." + *names->item(i),
				XmlText(b.attrs()->value("vprimitive")));
			s << stitle;

			s << db.include(key);
			sblob << s;
			chapter << *db.add(sblob);

			delete names->item(i);
		}

		doc << chapter;
		blob << doc;
		RepToHtmlFile::Location(db, blob, htmlFileName(String("everything") + scopeImage));
		res.append(db.add(blob));

		delete names;
	}
}

static
void renderToFile(BlobDb &db, const String &key) {
	if (const String fname = RepToHtmlFile::Location(key)) {
		clog << "creating: " << fname << endl;
		ofstream f(fname.cstr());
		if (Should(f)) {
			RepToHtmlFile r(db, &f, fname);
			r.renderReportBlob(db.get(key));
		}
	} else {
		clog << "internal_error: no location for " << key << endl;
	}
}

static
void buildReport(BlobDb &db) {
	clog << "building report" << endl;

	/* build first, then render so that all links are defined */
	Array<const ReportBlob*> blobs;

	buildEverything(db, blobs);
	blobs.append(buildFrontPage(db));
	blobs.append(buildOnePage(db));
	blobs.append(buildTrafficPage(db, TheTest->execScope()));
	blobs.append(buildRptmPage(db, TheTest->execScope()));
	blobs.append(buildSavingsPage(db, TheTest->execScope()));
	blobs.append(buildLevelsPage(db, TheTest->execScope()));
	blobs.append(buildAuthPage(db, TheTest->execScope()));
	blobs.append(buildErrorsPage(db, TheTest->execScope()));
	blobs.append(buildWorkloadPage(db, TheTest->execScope()));
	blobs.append(buildNotesPage(db));

	for (int i = 0; i < blobs.count(); ++i)
		renderToFile(db, blobs[i]->key());

	//doc.print(clog, "DOC: ");
	//db.print(clog, "DB:  ");
}

static
String guessLabel() {
	Array<String*> parts;
	Array<bool> ignoredParts;
	for (int i = 0; i < TheRepOpts.theFiles.count(); ++i) {
		String fname = *TheRepOpts.theFiles[i];
		int partIdx = 0;
		int skip = 0;
		while (const int pos = strcspn(fname.cstr()+skip, "-.:")) {
			const String part = fname(0, pos+1);
			if (i == 0) {
				parts.append(new String(part));
				ignoredParts.append(false);
			} else
			if (partIdx < parts.count() && part != *parts[partIdx]) {
				ignoredParts[partIdx] = true;
			}
			partIdx++;
			if (pos >= fname.len())
				break;
			fname = fname(pos+1, fname.len());
			skip = strspn(fname.cstr(), "-.:");
		}
	}

	String label;

	// cut the last part off because it is probably an extension
	const int lastIdx = parts.count() - 2;
	for (int p = 0; p <= lastIdx ; ++p) {
		if (!ignoredParts[p]) {
			const int len = p == lastIdx ?
				strcspn(parts[p]->cstr(), "-.:") : parts[p]->len();
			label += (*parts[p])(0, len);
		}
	}

	if (!label)
		label = "unlabeled";

	clog << "no test label specified, using '" << label << "'" << endl;

	while (parts.count()) delete parts.pop();

	return label;
}

static
void shutdownAtNew() {
	cerr << "error: ran out of RAM" << endl << xexit;
}

static
void configure() {
	const String label = TheRepOpts.theLabel ?
		(String)TheRepOpts.theLabel : guessLabel();

	if (!TheRepOpts.theRepDir)
		TheRepOpts.theRepDir.val(String("/tmp/polyrep/") + label);

	TheTest = new TestInfo(label);

	ReportFigure::TheBaseDir = TheRepOpts.theRepDir + "/figures";
	Should(::system((String("mkdir -p ") + TheRepOpts.theRepDir).cstr()) == 0);
	Should(::system((String("mkdir -p ") + ReportFigure::TheBaseDir).cstr()) == 0);

	Should(xset_new_handler(&shutdownAtNew));
}

static
void configureExecScope() {
	if (TheRepOpts.thePhases) {
		InfoScope scope;
		scope.name("baseline");
		scope.addSide("client");
		scope.addSide("server");
		for (int i = 0; i < TheRepOpts.thePhases.val().count(); ++i) {
			const String &name = *TheRepOpts.thePhases.val()[i];
			if (const SideInfo *side = TheTest->cltSideExists()) {
				if (!side->hasPhase(name)) {
					cerr
						<< "warning: ignoring '--phases "
						<< name << "' because client-side "
						<< "logs lack the named phase"
						<< endl;
					continue;
				}
			}
			if (const SideInfo *side = TheTest->srvSideExists()) {
				if (!side->hasPhase(name)) {
					cerr
						<< "warning: ignoring '--phases "
						<< name << "' because server-side "
						<< "logs lack the named phase"
						<< endl;
					continue;
				}
			}
			scope.addPhase(*TheRepOpts.thePhases.val()[i]);
		}
		if (!scope.phases().empty())
			TheTest->execScope(scope);
	}
}

int main(int argc, char *argv[]) {

	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheRepOpts);
	if (!cmd.parse(argc, argv) || !TheRepOpts.validate())
		return -1;

	configure();
	scanAll();
	configureExecScope();
	checkConsistency();

	BlobDb db;
	compileStats(db);
	buildReport(db);

	return 0;
}
