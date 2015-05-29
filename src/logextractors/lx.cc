
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include <fstream>
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"

#include "base/StatPhaseRec.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "base/polyOpts.h"
#include "base/CmdLine.h"
#include "base/opts.h"

#include "logextractors/LogIter.h"
#include "logextractors/LogCatFilter.h"
#include "logextractors/AggrInfoLoader.h"
#include "logextractors/matchAndPrint.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,     "help",         "list of options"),
			theVersOpt(this,     "version",      "package version info"),
			theOutFileName(this, "out <file>",   "redirect console output", "-"),
			theSideName(this,    "side <clt|srv|all>", "name of `side' to extract"),
			thePhases(this,      "phases <list>","names of phases to extract"),
			theObjects(this,     "objects <list>","names of objects to extract")
			{}

		virtual ostream &printAnonym(ostream &os) const;
		virtual bool parseAnonym(const Array<const char *> &opts);
		virtual bool canParseAnonym() const { return true; }

		virtual bool validate() const;

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theOutFileName;
		StrOpt theSideName;
		StrArrOpt thePhases;
		StrArrOpt theObjects;
		Array<String*> theFiles;
};

static MyOpts TheOpts;
static stringstream ReportBuf;


/* MyOpt */

bool MyOpts::validate() const {
	if (theSideName && theSideName != "clt" && theSideName != "srv" && theSideName != "all")
		cerr << "side name must be `clt' or `srv' or `all'; got: " << theSideName << endl;
	else
		return OptGrp::validate();
	return false;
}

ostream &MyOpts::printAnonym(ostream &os) const {
	return os << "<log_file_name> ...";
}

bool MyOpts::parseAnonym(const Array<const char *> &opts) {
	for (int i = 0 ; i < opts.count(); ++i)
		theFiles.append(new String(opts[i]));

	return theFiles.count() > 0;
}


/* local routines */

static
void configureLogs(int prec) {
	if (TheOpts.theOutFileName && TheOpts.theOutFileName != "-")
		redirectOutput(TheOpts.theOutFileName.cstr());

	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);

	configureStream(ReportBuf, prec);
}

static
void configure() {
	configureLogs(2);
}

static
void report(const AggrInfoLoader &ldr) {
	StatPhaseRec r;
	if (!ldr.aggregate(r)) {
		if (TheOpts.thePhases.val().count())
			cerr << "no matching phases found" << endl;
		else
			cerr << "no phase statistics found" << endl;
		exit(-1);
	}

	r.print(ReportBuf, "");

	ReportBuf << "log.count:\t " << TheOpts.theFiles.count() << endl;

	ReportBuf << ends;
	if (MatchAndPrint(cout, ReportBuf.str().c_str(), TheOpts.theObjects.val()) == 0) {
		cerr << "no logged objects matched any of the requested object names" << endl;
		exit(-2);
	}
}

int main(int argc, char *argv[]) {

	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	configure();

	int lc = lgcAll;
	if (TheOpts.theSideName == "clt")
		lc = lgcCltSide;
	else
	if (TheOpts.theSideName == "srv")
		lc = lgcSrvSide;

	LogCatFilter *filter = new LogCatFilter;
	filter->logCat(lc);

	AggrInfoLoader ldr(TheOpts.thePhases.val());
	ldr.filter(filter);

	for (int i = 0; i < TheOpts.theFiles.count(); ++i) {
		const String &fname = *TheOpts.theFiles[i];
		ILog log;
		if (fname == "-")
			log.stream("stdin", &cin);
		else
			log.stream(fname, (istream*)new ifstream(fname.cstr(), ios::binary|ios::in));

		for (LogIter li(&log); li; ++li)
			ldr.load(log, *li);
	}

	report(ldr);
	delete filter;

	return 0;
}
