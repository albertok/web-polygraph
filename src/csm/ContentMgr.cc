
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ContTypeStat.h"
#include "csm/ContentMgr.h"

ContentMgr TheContentMgr;


void ContentMgr::configure() {
	makeFakeContentType(TheForeignContentId, "foreign");
	makeFakeContentType(TheBodilessContentId, "bodiless");
	makeFakeContentType(TheUnknownContentId, "unknown");

	ContType::NoteNormalContentStart();
}

ContentCfg *ContentMgr::makeCfg() {
	return new ContentCfg(theCfgCounter++);
}

void ContentMgr::makeFakeContentType(int &id, const String &kind) {
	ContentCfg *const cfg = makeCfg();
	cfg->theKind = kind;
	id = cfg->id();
	ContType::Record(id, kind);
	theCfgs.append(cfg);
	theSyms.append(0);
}
