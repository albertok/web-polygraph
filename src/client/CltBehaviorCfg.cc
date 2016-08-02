
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/CltBehaviorCfg.h"
#include "csm/ContentMgr.h"
#include "pgl/ClientBehaviorSym.h"


CltBehaviorCfgs TheCltBehaviorCfgs;


void CltBehaviorCfg::configure(const ClientBehaviorSym *const aCltBehaviorSym) {
	Assert(!theRobot && aCltBehaviorSym);
	theRobot = aCltBehaviorSym;

	theRobot->recurRatio(theRecurRatio);
	configurePopModel();
	configureReqTypes();
	configureReqMethods();
	configureRanges();
}

void CltBehaviorCfg::configureReqTypes() {
	if (theRobot->haveReqTypes())
		CltCfg::configureReqTypes();
}

void CltBehaviorCfg::configureReqMethods() {
	if (theRobot->haveReqMethods())
		CltCfg::configureReqMethods();
}


/* CltBehaviorCfgs */

const CltBehaviorCfg *CltBehaviorCfgs::get(const int contentId) const {
	Assert(0 <= contentId && contentId < count());
	return item(contentId);
}

void CltBehaviorCfgs::configure() {
	Assert(empty());
	for (int i = 0; i < TheContentMgr.count(); ++i) {
		const ClientBehaviorSym *const sym =
			TheContentMgr.get(i)->clientBehaviorSym();
		if (sym) {
			CltBehaviorCfg *const cfg = new CltBehaviorCfg();
			cfg->configure(sym);
			append(cfg);
		} else
			append(0);
	}
}
