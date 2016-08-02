
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CONTENTMGR_H
#define POLYGRAPH__CSM_CONTENTMGR_H

#include "pgl/ContentSym.h"
#include "runtime/ConfigSymMgr.h"
#include "csm/ContentCfg.h"

class ContentCfg;
class ContentSym;

// aggregates information about content descriptions
// prevents creation of large number of identical descriptions

class ContentMgr: public ConfigSymMgr<ContentSym, ContentCfg> {
	public:
		ContentMgr(): theCfgCounter(0) {}
		void configure();

	protected:
		virtual ContentCfg *makeCfg();
		void makeFakeContentType(int &id, const String &kind);

		int theCfgCounter;
};

extern ContentMgr TheContentMgr;

#endif
