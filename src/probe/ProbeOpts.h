
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBEOPTS_H
#define POLYGRAPH__PROBE_PROBEOPTS_H

#include "base/opts.h"
#include "base/polyOpts.h"

class ProbeOpts: public OptGrp {
	public:
		ProbeOpts();

		virtual bool validate() const;

		virtual ostream &printAnonym(ostream &os) const;
		virtual bool parseAnonym(const Array<const char *> &opts);
		virtual bool canParseAnonym() const { return true; }

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		TimeOpt theDuration;
		TimeOpt theContTout;
		TimeOpt theStatExchTout;
		StrArrOpt theCltHosts;
		StrArrOpt theSrvHosts;
		StrOpt theConsFileName;
		FileScanOpt theFileScanner;
};

extern ProbeOpts TheProbeOpts;

#endif
