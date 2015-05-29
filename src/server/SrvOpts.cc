
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/SharedOpts.h"
#include "server/SrvOpts.h"

SrvOpts TheSrvOpts;

SrvOpts::SrvOpts():
	ignoreUrls(this,      "ign_urls <bool>",    "generate content regardless of the URLs", false) {
}

bool SrvOpts::validate() const {
	return OptGrp::validate();
}
