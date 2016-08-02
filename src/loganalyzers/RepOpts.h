
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_REPOPTS_H
#define POLYGRAPH__LOGANALYZERS_REPOPTS_H

#include "base/opts.h"
#include "base/polyOpts.h"

// options for the reporter
class RepOpts: public OptGrp {
	public:
		RepOpts(); // defaults

		virtual ostream &printAnonym(ostream &os) const;
		virtual bool parseAnonym(const Array<const char *> &opts);
		virtual bool canParseAnonym() const { return true; }

		virtual bool validate() const;
		
	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theLabel;
		StrArrOpt thePhases;
		StrOpt theDataDir;
		StrOpt theRepDir;
		StrOpt thePlotter;
		PtrArray<String*> theFiles;
};

extern RepOpts TheRepOpts;

#endif
