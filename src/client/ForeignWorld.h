
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_FOREIGNWORLD_H
#define POLYGRAPH__CLIENT_FOREIGNWORLD_H

#include "xstd/String.h"
#include "xstd/Array.h"
#include "runtime/ObjWorld.h"

// an iterator over a trace of foreign URLs
// preloads the trace in RAM at configure time
class ForeignWorld: public ObjWorld {
	public:
		static int TheWss; // all local foreign worlds

	public:
		ForeignWorld();

		void configure(const String &aName);

		void repeat(ObjId &oid, ObjSelector *sel);
		void produce(ObjId &oid, RndGen &rng);

	protected:
		void setType(ObjId &oid) const;
		void setUrl(ObjId &oid) const;

		String theName;
		Array<String*> theUrls;
};

#endif
