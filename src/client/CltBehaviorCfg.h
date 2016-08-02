
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CLTBEHAVIORCFG_H
#define POLYGRAPH__CLIENT_CLTBEHAVIORCFG_H

#include "client/CltCfg.h"

class ClientBehaviorSym;

class CltBehaviorCfg: public CltCfg {
	public:
		typedef bool (CltBehaviorCfg::*Predicate)() const;

		void configure(const ClientBehaviorSym *const aCltBehaviorSym);

		bool havePopModel() const { return thePopModel; }
		bool haveRanges() const { return theRangeSel; }
		bool haveRecurRatio() const { return theRecurRatio >= 0; }
		bool haveReqMethods() const { return theReqMethodSel; }
		bool haveReqTypes() const { return theReqTypeSel; }

	protected:
		void configureReqTypes();
		void configureReqMethods();
};

class CltBehaviorCfgs: protected PtrArray<const CltBehaviorCfg*> {
	public:
		const CltBehaviorCfg *get(const int contentId) const;
		void configure();
};

extern CltBehaviorCfgs TheCltBehaviorCfgs;

#endif
