
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_OBJSELECTOR_H
#define POLYGRAPH__RUNTIME_OBJSELECTOR_H

#include "base/ObjId.h"

class RndGen;
class ObjId;

// base class for various object selectors (e.g., PopModel)
class ObjSelector {
	public:
		virtual ~ObjSelector() {}

		// selects oid, adjusts for wss and hot set position
		virtual void choose(Counter lastOid, Counter wsCap, Counter hotSetPos, ObjId &oid) = 0;
};

#endif
