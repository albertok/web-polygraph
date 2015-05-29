
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include <fstream>
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"

#include "base/CmdLine.h"
#include "base/opts.h"

#include "base/ILog.h"
#include "base/polyOpts.h"
#include "base/polyLogTags.h"
#include "xstd/gadgets.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,      "help",           "list of options"),
			theVersOpt(this,      "version",        "package version info"),
			theOutFileName(this,  "out <file>",     "redirect console output", "-")
			{}

		virtual ostream &printAnonym(ostream &os) const;
		virtual bool parseAnonym(const Array<const char *> &opts);
		virtual bool canParseAnonym() const { return true; }

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theOutFileName;

		Array<String*> theFileNames;
};

static MyOpts TheOpts;
static ILog *TheInLog = 0;


ostream &MyOpts::printAnonym(ostream &os) const {
	return os << "<log_file_name> ...";
}

bool MyOpts::parseAnonym(const Array<const char *> &opts) {
	for (int i = 0 ; i < opts.count(); ++i)
		theFileNames.append(new String(opts[i]));
	// add default fname if none are specified
	if (!theFileNames.count())
		theFileNames.append(new String("-"));
	return theFileNames.count() > 0;
}


static
ostream &loghere(ostream &os) {
	return os << setw(7) << TheInLog->progress().time() << "| ";
}

static
void doEntry(const LogEntryPx &px) {
	if (px.theTag == lgComment) {
		String s;
		TheInLog->gets(s);
		cout << loghere << s << endl;
	}
}

static
void configureLogs(int prec) {
	if (TheOpts.theOutFileName && TheOpts.theOutFileName != "-")
		redirectOutput(TheOpts.theOutFileName.cstr());

	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);
}

// set some general stuff and
// propogate cmd line options to corresponding objects
static
void configure() {

	configureLogs(2);
}

int main(int argc, char *argv[]) {

	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate() || !TheOpts.validate())
		return -1;

	configure();

	for (int i = 0; i < TheOpts.theFileNames.count(); ++i) {
		const String &fname = *TheOpts.theFileNames[i];
		istream *is = new ifstream(fname.cstr(), ios::binary|ios::in);
		TheInLog = new ILog();
		TheInLog->stream(fname.cstr(), is);

		if (TheOpts.theFileNames.count() > 1)
			cout << endl << "==> " << fname << " <==" << endl << endl;

		while (const LogEntryPx prefix = TheInLog->begEntry()) {
			doEntry(prefix);
			TheInLog->endEntry();
		}

		delete TheInLog;
		delete is;
	}

	return 0;
}
