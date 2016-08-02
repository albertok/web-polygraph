
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "loganalyzers/CompOpts.h"

CompOpts TheCompOpts;

CompOpts::CompOpts():
	theHelpOpt(this, "help",               "list of options"),
	theVersOpt(this, "version",            "package version info"),
	theDelta(this,   "delta <double>",     "maximum value difference to ignore", 0.0),
	thePhases(this,  "phases <list>",      "names of phases for executive summary"),
	theCompDir(this, "report_dir <dir>",   "report's root directory")
{
}

bool CompOpts::validate() const {
	return OptGrp::validate();
}

ostream &CompOpts::printAnonym(ostream &os) const {
	return os << "<report_file_name> ...";
}

bool CompOpts::parseAnonym(const Array<const char *> &opts) {
	for (int i = 0 ; i < opts.count(); ++i)
		theReports.append(new String(opts[i]));
	return theReports.count() > 0;
}
