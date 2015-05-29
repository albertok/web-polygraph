
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

		// rnd distr of array idxes that matches item probabilities
		RndDistr *makeSelector(const String &name);
		void copyProbs(Array<double> &res) const;

		virtual void forEach(Visitor &v) const;

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

	protected:
		Array<SynSym*> theItems;   // array [top-level] members
		String theItemType;        // set when known

		Array<double> theProbs;     // explicit probs for each item
		bool nested; // this array contains other containers
		mutable bool warnedBadProbs;
};


/* template to transform array symbol into arrays of symbols/items
 * the only reason to use a template is to avoid a[i] casting
 * think: maybe the array symbol should be a template instead??
 */

#ifdef COMPILER_CAN_HANDLE_NONTRIVIAL_TEMPLATES

	template <class Item>
	int ArraySymExportT(const ArraySym &as, const char *typeName, 
		Array<Item*> &arr, Array<double> *probs = 0) {

		arr.stretch(as.count());
		if (probs)
			as.copyProbs(*probs);
		for (int i = 0; i < as.count(); ++i) {
			Item *item = &((Item&)as[i]->cast(typeName));
			arr.append(item);
		}
		return arr.count();
	}

#else

	/* macro for template-challanged compilers */
#	define ArraySymExportM(Item, as, typeName, arr) {          \
		                                                       \
		(arr).stretch((as).count());                           \
		for (int i = 0; i < (as).count(); ++i) {               \
			Item *item = &((Item&)(as)[i]->cast(typeName));    \
			(arr).append(item);                                \
		}                                                      \
	}

	/* macro for template-challanged compilers */
#	define ArraySymExportMP(Item, as, typeName, arr, probs)  { \
		                                                       \
		(arr).stretch((as).count());                           \
		if (probs)                                             \
			(probs)->stretch((as).count());                    \
		for (int i = 0; i < (as).count(); ++i) {               \
			Item *item = &((Item&)(as)[i]->cast(typeName));    \
			(arr).append(item);                                \
			if (probs)                                         \
				(probs)->append((as).prob(i));                 \
		}                                                      \
	}                                                          

#endif /* COMPILER_CAN_HANDLE_NONTRIVIAL_TEMPLATES */

#endif
