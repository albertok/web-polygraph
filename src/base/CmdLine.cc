
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Assert.h"
#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "base/Opt.h"
#include "base/CmdLine.h"


/* CmdLine utils */

static
bool namesEq(const char *name, const char *canonic) {
	if (!name || !canonic)
		return false;
	
	// strip leading '-'s
	while (*name == '-') name++;
	while (*canonic == '-') canonic++;

	if (!*name)
		return false;

	//clog << here << "comapring: " << name << " ? " << canonic << endl;

	return strcasecmp(name, canonic) == 0;
}

// returns val if it looks like a value rather than an option name
static
const char *peekVal(const String &val) {
	if (!val || val[0] != '-')
		return val.cstr();
	if (isdigit(val[1]))   // negative integer's are valid values
		return val.cstr();
	if (val == "-") // string '-' is a valid value
		return val.cstr();
	return 0;
}


/* CmdLine */

CmdLine::CmdLine(): theAnonymParser(0), thePrgName("?") {
}

CmdLine::~CmdLine() {
	while (theArgs.count()) delete theArgs.pop();
}

void CmdLine::configure(const Array<OptGrp*> &groups) {
	// absorb all options
	for (int g = 0; g < groups.count(); ++g) {
		OptGrp *gr = groups[g];

		if (gr->canParseAnonym()) {
			Assert(!theAnonymParser);
			theAnonymParser = gr;
		}

		for (int i = 0; i < gr->count(); ++i) {
			Opt *opt = gr->item(i);
			opt->cmdLine(this);
			theOpts.append(opt);
		}
	}
}

bool CmdLine::parse(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i)
		theArgs.append(new String(argv[i]));

	if (theArgs.count())
		thePrgName = *theArgs[0];

	// XXX: kludge; we should probably use a no-param option or something
	if (theArgs.count() <= 1) {
		usage(cerr);
		return false;
	}

	int argi = 0; // warning: used till the end of this function
	for (argi = 1; argi < theArgs.count() && theArgs[argi]->cstr()[0] == '-'; ++argi) {
		const char *val = (argi+1 < theArgs.count()) ? 
			peekVal(*theArgs[argi+1]) : 0;
		if (!parse(*theArgs[argi], val))
			return false;
		if (val) // skip option's value if any
			argi++;
	}

	if (argi < theArgs.count() && !theAnonymParser) {
		cerr << thePrgName << ": expected --option, found '" << *theArgs[argi] << "'" << endl;
		return false;
	}

	// handle anonymous options
	if (theAnonymParser) {
		Array<const char *> anon;
		while (argi < theArgs.count()) { anon << theArgs[argi++]->cstr(); }
		// always call this: anonymous options may have their own defaults!
		return theAnonymParser->parseAnonym(anon);
	}

	return true;
}

bool CmdLine::parse(const String &name, const String &val) {
	for (int i = 0; i < theOpts.count(); ++i) {
		if (namesEq(name.cstr(), theOpts[i]->name().cstr()))
			return theOpts[i]->set(name, val);
	}
	cerr << thePrgName << ": unknown option: '"
		<< name << ' ' << val << "'" << endl;
	return false;
}

void CmdLine::usage(ostream &os) const {
	os << "Usage: "	<< thePrgName << " [--option ...]";
	if (theAnonymParser) theAnonymParser->printAnonym(os << ' ');
	os << endl << endl;

	os << "Options:" << endl;
	for (int i = 0; i < theOpts.count(); ++i) {
		const Opt *o = theOpts[i];
		const int plen = o->name().len() + o->type().len() + 1;
		os
			<< "  --" << o->name() << ' ' << o->type()
			<< setw(20-plen) << ""
			<< "  " << o->descr()
			<< endl;
	}
}

void CmdLine::report(ostream &os) const {
	reportRaw(os);
	os << endl;
	reportParsed(os);
	os << endl;
}

void CmdLine::reportRaw(ostream &os) const {
	os << "Command:";
	for (int i = 0; i < theArgs.count(); ++i)
		os << ' ' << *theArgs[i];
}

void CmdLine::reportParsed(ostream &os) const {
	os << "Configuration:";
	for (int i = 0; i < theOpts.count(); ++i) {
		if (!theOpts[i]->visible())
			continue;
		os << endl;
		const int plen = theOpts[i]->name().len() + 1;
		os << "\t" << theOpts[i]->name() << ':' << setw(20-plen) << "";
		theOpts[i]->report(os);
	}
}
