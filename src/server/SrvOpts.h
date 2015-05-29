
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__SERVER_SRVOPTS_H
#define POLYGRAPH__SERVER_SRVOPTS_H

#include "base/opts.h"

// client options
class SrvOpts: public OptGrp {
	public:
		SrvOpts(); // defaults
		virtual bool validate() const;
		
	public:
		BoolOpt ignoreUrls;
};

extern SrvOpts TheSrvOpts;

#endif
