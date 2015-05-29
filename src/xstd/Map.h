
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_MAP_H
#define POLYGRAPH__XSTD_MAP_H

#include "xstd/String.h"
#include "xstd/Array.h"


// a simple add-only (string -> any) map

// map element
template <class Val>
class MapItem {
	public:
		MapItem();
		MapItem(const String &aKey, const Val &aVal);

	public:
		String *theKey;
		Val theVal;
};

// the map itself
template <class Val>
class Map {
	public:
		typedef MapItem<Val> Item;

	public:
		virtual ~Map();

		void reset();

		int count() const { return theItems.count(); }

		const Item &item(int idx) const { return theItems[idx]; }
		const Item &operator [](int idx) const { return item(idx); }

		const String &keyAt(int idx) const { return *item(idx).theKey; }
		Val &valAt(int idx) { return theItems[idx].theVal; }
		const Val &valAt(int idx) const { return theItems[idx].theVal; }

		void add(const String &key, const Val &val);
		bool find(const String &key, Val &val);
		bool find(const String &key, Val &val) const;
		virtual Val *valp(const String &key);

	protected:
		virtual void noteAdd(int) {} // hook
		virtual void noteDel(int) {} // hook

	protected:
		Array<Item> theItems;
};

// map for pointers to objects
template <class Val>
class PtrMap: public Map<Val> {
	public:
		virtual ~PtrMap();
};

/* implementaion of in-lined methods */

/* MapItem */

template <class Val>
inline
MapItem<Val>::MapItem(): theKey(0) {
}

template <class Val>
inline
MapItem<Val>::MapItem(const String &aKey, const Val &aVal):
	theKey(new String(aKey)), theVal(aVal) {
}


/* Map */

template <class Val>
inline
Map<Val>::~Map() {
	reset();
}

template <class Val>
inline
void Map<Val>::reset() {
	while (count()) {
		noteDel(count()-1);
		delete theItems.pop().theKey;
	}
}

template <class Val>
inline
void Map<Val>::add(const String &key, const Val &val) {
	theItems.append(Item(key, val));
	noteAdd(count()-1);
}

template <class Val>
inline
Val *Map<Val>::valp(const String &key) {
	for (int i = 0; i < count(); ++i) {
		if (keyAt(i) == key)
			return &valAt(i);
	}
	return 0;
}

template <class Val>
inline
bool Map<Val>::find(const String &key, Val &v) {
	if (Val *vp = valp(key)) {
		v = *vp;
		return true;
	}
	return false;
}

template <class Val>
inline
bool Map<Val>::find(const String &key, Val &v) const {
	// XXX: the cast to non-const is a kludge to have one copy of valp()
	if (Val *vp = ((Map<Val>*)this)->valp(key)) { 
		v = *vp;
		return true;
	}
	return false;
}

template <class Val>
inline
PtrMap<Val>::~PtrMap() {
	for (int i = 0; i < this->count(); ++i)
		delete this->valAt(i);
}

#endif
