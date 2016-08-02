
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLARRAYSYM_H
#define POLYGRAPH__PGL_PGLARRAYSYM_H

#include "xstd/Array.h"
#include "pgl/PglContainerSym.h"

class RndDistr;
class String;

// array of PglArrayItems with optional probabilities
class ArraySym: public ContainerSym {
	public:
		static String TheType;

	public:
		ArraySym();
		ArraySym(const String &anItemType);
		virtual ~ArraySym();

		virtual bool isA(const String &type) const;

		virtual bool empty() const;
		virtual int count() const;
		virtual bool probsSet() const;
		virtual const SynSym *itemProb(int offset, double &prob) const;

		virtual const SynSym *item(int idx) const;
		double prob(int idx) const;

		void reserve(int minCount);
		void add(const SynSym &s, double prob = -1);
		bool cadd(const SynSym &s, double prob = -1);
		bool append(const ArraySym &arr);

		// flattens and exports Items
		template <class Item>
		int exportA(Array<Item*> &arr) const;

		// exports Arrays of Items
		template <class Item>
		bool exportNestedArrays(Array< Array<Item*> > &arr) const;

		// rnd distr of array idxes that matches item probabilities
		RndDistr *makeSelector(const String &name);
		void copyProbs(Array<double> &res) const;

		virtual void forEach(Visitor &v, RndGen *const rng = 0) const;

		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
		virtual ArraySym *create(const String &itemType) const;

		int itemCountAt(int idx) const;
		bool itemProbsSetAt(int idx) const;
		const SynSym *itemProbAt(int idx, int offset, double &prob) const;

		double actualProb(double p) const;
		double explProb(int firstLevelOff) const;

		// a helper for the exportNestedArrays() template; does no type casting
		static bool GetNestedArraysFromItem(Array<const ArraySym*> &arrays, const SynSym *item);

	protected:
		Array<SynSym*> theItems;   // array [top-level] members
		const String theItemType;  // set when known

		Array<double> theProbs;     // explicit probs for each item
		bool nested; // this array contains other containers
		mutable bool warnedBadProbs;
};


/* template to transform array symbol into arrays of symbols/items
 * the only reason to use a template is to avoid a[i] casting
 * think: maybe the array symbol should be a template instead?? */
template <class Item>
int ArraySym::exportA(Array<Item*> &arr) const {
	arr.stretch(count());
	for (int i = 0; i < count(); ++i) {
		Item *const item = &((Item&)(*this)[i]->cast(Item::TheType));
		arr.append(item);
	}
	return arr.count();
}

template <class Item>
bool ArraySym::exportNestedArrays(Array< Array<Item*> > &arr) const {
	if (!nested)
		return false;

	Array<const ArraySym*> arrays;
	for (int i = 0; i < theItems.count(); ++i) {
		if (!GetNestedArraysFromItem(arrays, theItems[i]))
			return false;
	}

	for (int i = 0; i < arrays.count(); ++i) {
		const ArraySym &a = *arrays[i];
		Array<Item*> arrItem(a.count());
		for (int j = 0; j < a.count(); ++j) {
			Item *const item = &((Item&)a[j]->cast(Item::TheType));
			arrItem.append(item);
		}
		arr.append(arrItem);
	}
	return true;
}

#endif
