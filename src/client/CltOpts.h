
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CLTOPTS_H
#define POLYGRAPH__CLIENT_CLTOPTS_H

#include "base/opts.h"

// client options
class CltOpts: public OptGrp {
	public:
		CltOpts(); // defaults

		virtual bool validate() const;

	public:
		NetAddrOpt theProxyAddr;
		StrArrOpt theLoadableModules;

		TimeOpt theIcpTout;

		BoolOpt ignoreFalseHits;
		BoolOpt ignoreBadContTags;
		BoolOpt printFalseMisses;
};

extern CltOpts TheCltOpts;

#endif
