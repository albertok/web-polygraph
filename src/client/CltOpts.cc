
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/CltOpts.h"

#include "runtime/globals.h"


CltOpts TheCltOpts;


CltOpts::CltOpts():
	theProxyAddr(this,      "proxy <host:port>",   "proxy location", NetAddr()),

	theLoadableModules(this,"loadable_modules <filenames>", "dynamic modules to load"),

	theIcpTout(this,        "icp_tout <time>",     "how long to wait for an ICP_HIT reply", Time::Sec(2)),

	ignoreFalseHits(this,   "ign_false_hits <bool>","ignore false hits", true),
	ignoreBadContTags(this, "ign_bad_cont_tags <bool>","ignore bad content tags", false),
	printFalseMisses(this,  "prn_false_misses <bool>","dump rep headers of false misses")
{
}

bool CltOpts::validate() const {
	return OptGrp::validate();
}
