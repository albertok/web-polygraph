
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CLTDATAFILTERREGISTRY_H
#define POLYGRAPH__CLIENT_CLTDATAFILTERREGISTRY_H

#include "runtime/ModuleRegistry.h"
#include "runtime/DataFilter.h"

class CltXact;

// registration handler for client-side data filters
class CltDataFilterRegistry: public ModuleRegistry< DataFilter<CltXact*> > {
	public:
		typedef CltXact* Producer;
		typedef DataFilter<Producer> Filter;

	public:
		CltDataFilterRegistry();
		void apply(CltXact *p, IOBuf &buf); // applies to all
};

extern CltDataFilterRegistry &TheCltDataFilterRegistry();

LIB_INITIALIZER(CltDataFilterRegistryInit)

#endif
