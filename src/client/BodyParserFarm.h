
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_BODYPARSERFARM_H
#define POLYGRAPH__CLIENT_BODYPARSERFARM_H

#include "runtime/Farm.h"
#include "client/BodyParser.h"

// we need a farm that can supply "*BodyParser" without knowing the "*" part

class BodyParserFarm {
	public:
		virtual ~BodyParserFarm() {}

		virtual int capacity() const = 0;
		virtual void limit(int limit) = 0;

		virtual BodyParser *get() = 0;
		virtual void put(BodyParser *x) = 0;
};

// use this template to build real farms
template <class Parser>
class BodyParserFarmT: public BodyParserFarm {
	public:
		BodyParserFarmT() { theFarm = new ObjFarm<Parser>; }
		virtual ~BodyParserFarmT() { delete theFarm; } 

		virtual int capacity() const { return theFarm->capacity(); }
		virtual void limit(int l) { theFarm->limit(l); } 

		virtual BodyParser *get() { return getTyped(); }
		virtual void put(BodyParser *x) { x->reset(); theFarm->put((Parser*)x); } 

		Parser *getTyped() { return theFarm->get(); } 

	protected:                                                
		ObjFarm<Parser> *theFarm; 
};

#endif
