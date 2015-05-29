
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_STATSSAMPLECFG_H
#define POLYGRAPH__PGL_STATSSAMPLECFG_H

#include "xstd/String.h"

// a record describing a single stats sample

class StatsSampleCfg {
	public:
		StatsSampleCfg(): capacity(0) {}

	public:
		String name;  // sample name
		Time start;   // when to start sampling (relative to run start)
		int capacity; // entries in this sample
};

#endif
