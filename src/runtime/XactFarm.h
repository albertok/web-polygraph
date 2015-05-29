
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_XACTFARM_H
#define POLYGRAPH__RUNTIME_XACTFARM_H

#include "runtime/Farm.h"

// we need a farm that can supply *Xact without owner knowing the "*" part,
// where *Xact is assumed to be Clt/SrvXact or derived from them

// interface for users
template <class Side>
class XactFarm {
	public:
		virtual ~XactFarm() {}
		virtual void limit(int limit) = 0;
		virtual Side *get() = 0;
		virtual void put(Side *x) = 0;
};

// use this template to build real farms
template <class Side, class Xact>
class XactFarmT: public XactFarm<Side> {       
	public:
		XactFarmT() { theFarm = new ObjFarm<Xact>; }
		virtual ~XactFarmT() { delete theFarm; } 

		virtual void limit(int l) { theFarm->limit(l); } 
		virtual Side *get() { return theFarm->get(); } 
		virtual void put(Side *x) { theFarm->put((Xact*)x); } 

	protected:                                                
		ObjFarm<Xact> *theFarm; 
};

#endif
