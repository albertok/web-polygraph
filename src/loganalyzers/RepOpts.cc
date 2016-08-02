
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "loganalyzers/RepOpts.h"

RepOpts TheRepOpts;

RepOpts::RepOpts():
	theHelpOpt(this, "help",               "list of options"),
	theVersOpt(this, "version",            "package version info"),
	theLabel(this,   "label <str>",        "test label"),
	thePhases(this,  "phases <list>",      "names of phases for executive summary"),
	theDataDir(this, "data_dir <dir>",     "data directory where stylesheets and scripts are installed", DATA_DIR),
	theRepDir(this,  "report_dir <dir>",   "report's root directory"),
	thePlotter(this, "plotter <exe_file>", "gnuplot location", "gnuplot")
{
}

bool RepOpts::validate() const {
	if (!thePlotter)
		cerr << "plotter location must be specified" << endl;
	else
		return OptGrp::validate();
	return false;
}

ostream &RepOpts::printAnonym(ostream &os) const {
	return os << "<log_file_name> ...";
}

bool RepOpts::parseAnonym(const Array<const char *> &opts) {
	for (int i = 0 ; i < opts.count(); ++i)
		theFiles.append(new String(opts[i]));
	return theFiles.count() > 0;
}
