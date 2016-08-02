
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_FARM_H
#define POLYGRAPH__RUNTIME_FARM_H

#include "xstd/Array.h"

// a collection of Items that configures and recycles its members
// current recycling order is LIFO
// note <Item> -> <Item*> migration
template <class Item>
class Farm: public Array<Item*> {
	public:
		Farm(): theOutLevel(0) {}
		virtual ~Farm() {}

		void limit(int limit) { this->stretch(limit); }
		void populate(int count) { while (count-- > 0) put(gen()); }

		int inLevel() const { return this->count(); }      // stored
		int outLevel() const { return theOutLevel; } // out there

		Item *getDirty() { theOutLevel++; return this->empty() ? this->gen() : this->pop(); }
		Item *getClean() { return this->clean(getDirty()); }
		Item *get() { return this->getClean(); }

		void put(Item *i) { theOutLevel--; this->full() ? this->overflow(i) : this->push(clean(i)); }

	protected:
		virtual Item *gen() = 0;
		virtual Item *clean(Item *i) const { return i; }
		virtual void destroy(Item *i) = 0;

		virtual void overflow(Item *i) { destroy(i); }

	protected:
		int theOutLevel;    // items out there
};

// Fixed capacity Farm for "objects" (new/reset/delete)
template <class Item>
class ObjFarm: public Farm<Item> {
	public:
		ObjFarm() {}
		ObjFarm(const int n) { this->limit(n); }
		virtual ~ObjFarm() { while (!this->empty()) this->destroy(this->get()); }

	protected:
		virtual Item *gen() { return new Item(); }
		virtual Item *clean(Item *i) const { i->reset(); return i; }
		virtual void destroy(Item *i) { delete i; }
};

#endif
