
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ContTypeStat.h"
#include "csm/ContentMgr.h"

ContentMgr TheContentMgr;


ContentMgr::ContentMgr():
	theCfgCounter(0), theNormalContentStart(0), didInit(false) {
}

void ContentMgr::init() {
	makeFakeContentType(TheForeignContentId, "foreign");
	makeFakeContentType(TheBodilessContentId, "bodiless");
	makeFakeContentType(TheUnknownContentId, "unknown");

	theNormalContentStart = theCfgCounter;
}

ContentCfg *ContentMgr::makeCfg() {
	if (!didInit) {
		didInit = true;
		init();
	}

	ContentCfg *cfg = new ContentCfg(theCfgCounter++);
	return cfg;
}

void ContentMgr::makeFakeContentType(int &id, const String &kind) {
	ContentCfg *const cfg = makeCfg();
	cfg->theKind = kind;
	id = cfg->id();
	ContTypeStat::RecordKind(id, kind);
	theCfgs.append(cfg);
	theSyms.append(0);
}
