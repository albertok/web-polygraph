
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/CltDataFilterRegistry.h"


static CltDataFilterRegistry *TheRegistry = 0;


CltDataFilterRegistry::CltDataFilterRegistry():
	ModuleRegistry< DataFilter<CltXact*> >("client-side data filters") {
}

void CltDataFilterRegistry::apply(CltXact *xact, IOBuf &buf) {
	for (int i = 0; i < theModules.count(); ++i)
		theModules[i]->apply(xact, buf);
}

CltDataFilterRegistry &TheCltDataFilterRegistry() {
	Assert(TheRegistry);
	return *TheRegistry;
}

/* initialization */

int CltDataFilterRegistryInit::TheUseCount = 0;

void CltDataFilterRegistryInit::init() {
	TheRegistry = new CltDataFilterRegistry;
}

void CltDataFilterRegistryInit::clean() {
	delete TheRegistry;
	TheRegistry = 0;
}
