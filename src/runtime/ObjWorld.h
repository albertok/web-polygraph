
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_OBJWORLD_H
#define POLYGRAPH__RUNTIME_OBJWORLD_H

#include "base/UniqId.h"
#include "runtime/HotSet.h"

class RndGen;
class ObjId;
class ObjSelector;
class OBStream;
class IBStream;

class ObjWorld {
	public:
		ObjWorld();

		void reset();

		operator void*() const { return id() ? (void*)-1 : 0; }
		const UniqId &id() const { return theId; }
		int size() const { return theSize; }
		int wss() const { return theWss; }
		int hotPos() const { return theHotSet.pos(); }

		bool newer(const ObjWorld &w) const;

		bool canRepeat() const;
		bool canProduce() const;

		void repeat(ObjId &oid, ObjSelector *sel);
		void produce(ObjId &oid, RndGen &rng);

		void freezeWss();

		void store(OBStream &os) const;
		void load(IBStream &is);

		void id(const UniqId &anId) { theId = anId; }
		void size(int aSize) { theSize = aSize; }
		void wss(int aWss) { theWss = aWss; }
		void hotPos(int aPos) { theHotSet.pos(aPos); }

		ostream &print(ostream &os) const;

		bool parse(const char *buf, const char *end);

	protected:
		void produced();
		void incWss(int &count);
		void calcPrefix(ObjId &oid);

	protected:
		UniqId theId;
		int theSize;      // number of objects in the world
		int theWss;       // working set size
		HotSet theHotSet; // hot subset of wss
};

inline
ostream &operator <<(ostream &os, const ObjWorld &w) {
	return w.print(os);
}

#endif
