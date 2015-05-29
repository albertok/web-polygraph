
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ObjId.h"
#include "csm/ContentMgr.h"
#include "csm/ContentCfg.h"
#include "csm/ContentTypeIdx.h"


void ContentTypeIdx::configure(const Array<ContentCfg*> &ccfgs) {
	stretch(ccfgs.count());
	for (int i = 0; i < ccfgs.count(); ++i)
		append(ccfgs[i]->id());
}

bool ContentTypeIdx::hasContType(int tid) const {
	for (int i = 0; i < count(); ++i) {
		if (item(i) == tid)
			return true;
	}
	return false;
}

bool ContentTypeIdx::findEmbedContType(const Area &category, int &etype) const {
	// first "kind" match wins; also remember first MIME type match
	int mimeTid = -1;
	for (int i = 0; i < count(); ++i) {
		const int tid = item(i);
		const ContentCfg *cfg = TheContentMgr.get(tid);
		if (cfg->theKind.find(category) == 0) {
			mimeTid = tid;
			break;
		}
		if (mimeTid < 0 && cfg->theMimeType.find(category) == 0)
			mimeTid = tid;
	}

	if (mimeTid >= 0) {
		etype = mimeTid;
		return true;
	}

	return false;
}
