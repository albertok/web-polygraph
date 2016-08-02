
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_COMPOPTS_H
#define POLYGRAPH__LOGANALYZERS_COMPOPTS_H

#include "base/opts.h"
#include "base/polyOpts.h"

// options for the reporter
class CompOpts: public OptGrp {
	public:
		CompOpts(); // defaults

		virtual ostream &printAnonym(ostream &os) const;
		virtual bool parseAnonym(const Array<const char *> &opts);
		virtual bool canParseAnonym() const { return true; }

		virtual bool validate() const;
		
	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		DblOpt theDelta;
		StrArrOpt thePhases;
		StrOpt theCompDir;
		Array<String*> theReports;
};

extern CompOpts TheCompOpts;

#endif
