
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_POPMODEL_H
#define POLYGRAPH__RUNTIME_POPMODEL_H

#include "runtime/ObjSelector.h"

class RndGen;
class PopDistr;
class PopModelSym;

// popularity model

class PopModel: public ObjSelector {
	public:
		PopModel();

		void configure(const PopModelSym *cfg);

		// selects oid, adjusts for wss and hot set position
		virtual void choose(int lastOid, int wsCap, int hotSetPos, ObjId &oid);

	protected:
		void pickBest(int nameBeg, int nameEnd, ObjId &oid);

	protected:
		PopDistr *theDistr;
		double theHotSetProb;  // prob of selecting a hot object
		double theHotSetFrac;  // hot_set_size/wss
		double theBhrDiscr;    // probability of BHR discrimination
};

#endif
