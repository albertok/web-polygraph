
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"

#include "xstd/TblDistr.h"
#include "base/StringArray.h"
#include "base/RndPermut.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglClonerSym.h"

#include "xstd/rndDistrs.h"
#include "xstd/gadgets.h"


String ArraySym::TheType = "Array";


ArraySym::ArraySym(): ContainerSym(TheType), nested(false),
	warnedBadProbs(false) {
}

ArraySym::ArraySym(const String &anItemType): ContainerSym(TheType),
	theItemType(anItemType), nested(false), warnedBadProbs(false) {
}

ArraySym::~ArraySym() {
	while (theItems.count())
		delete theItems.pop();
}

bool ArraySym::isA(const String &type) const {
	if (ContainerSym::isA(type))
		return true;

	if (theItemType) {
		if (const char *p = type.str("[]")) {
			return type.cmp(theItemType, p - type.cstr()) == 0;
		}
	}

	return false;
}

SynSym *ArraySym::dupe(const String &type) const {
	String itemType;
	if (const char *p = type.str("[]"))
		itemType = type(0, p-type.cstr());
	else {
		if (!isA(type))
			return 0;
		itemType = theItemType;
	}

	ArraySym *clone = create(itemType);
	clone->warnedBadProbs = warnedBadProbs;

	if (clone->append(*this))
		return clone;

	delete clone;
	return 0;
}

ArraySym *ArraySym::create(const String &itemType) const {
	return new ArraySym(itemType);
}

// optimizes Container::empty()
bool ArraySym::empty() const {
	if (!nested)
		return !theItems.count();

	bool isEmpty = true;
	for (int idx = 0; isEmpty && idx < theItems.count(); ++idx) {
		if (theItems[idx]->isA(ContainerSym::TheType))
			isEmpty = ((const ContainerSym&)theItems[idx]->cast(ContainerSym::TheType)).empty();
		else
			isEmpty = false;
	}
	return isEmpty;
}

// optimizes Container::count()
int ArraySym::count() const {
	if (!nested)
		return theItems.count();

	int cnt = 0;
	for (int i = 0; i < theItems.count(); ++i) {
		cnt += itemCountAt(i);
	}
	return cnt;
}

const SynSym *ArraySym::item(int idx) const {
	if (!nested)
		return theItems[idx];

	double prob = -1;
	return itemProb(idx, prob);
}

double ArraySym::prob(int idx) const {
	double p = -1;
	itemProb(idx, p);
	Assert(p >= 0);
	return p;
}

double ArraySym::explProb(int firstLevelOff) const {
	return firstLevelOff < theProbs.count() ?
		theProbs[firstLevelOff] : (double)-1;
}

// adjusts assigned probability (or -1) into actual probability
// computes and uses defaults (if -1) or corrects for user-distribution errors
double ArraySym::actualProb(double p) const {
	Assert(theItems.count());

	// collect info about current probs
	int setCount = 0;
	double setSum = 0;
	for (int i = 0; i < theProbs.count(); ++i) {
		if (theProbs[i] >= 0) {
			setSum += theProbs[i];
			setCount++;
		}
	}

	if (setSum <= 0.99 && setCount == theItems.count()) {
		if (!warnedBadProbs) {
			print(cerr << loc(), String()) << endl;
			cerr << loc() << "explicit item probabilities in the array above add up to " 
				<< (100*setSum) << "% (less than 100%) and no wild-card entries are present;"
				<< " missing percents will be spread out proportionally among array entries" << endl;
			warnedBadProbs = true;
		}
		Assert(p >= 0);
		return setSum > 0 ? (p/setSum) : (1.0/setCount);
	}

	if (1.01 <= setSum) {
		if (!warnedBadProbs) {
			print(cerr << loc(), String()) << endl;
			cerr << loc() << "explicit array probabilities in the array above add up to " 
				<< (100*setSum) << "% (more than 100%); "
				<< "explicit probabilities will be adjusted proportionally to specified values" << endl;
			warnedBadProbs = true;
		}
		return p >= 0 ? (p/setSum) : 0;
	}

	if (p < 0)
		return (1.0 - setSum) / (theItems.count() - setCount);

	return p;
}

bool ArraySym::append(const ArraySym &arr) {
	const int oldCount = theItems.count();
	const bool oldNested = nested;
	for (int i = 0; i < arr.theItems.count(); ++i) {
		if (!cadd(*arr.theItems[i], arr.explProb(i))) {
			// restore old state
			while (theItems.count() > oldCount)
				delete theItems.pop();
			nested = oldNested;
			return false;
		}
	}
	return true;
}

bool ArraySym::cadd(const SynSym &s, double p) {
	//if (theItems.count() && s.isA(ContainerSym::TheType) && p < 0) {
	//	cerr << s.loc() << "warning: use binary plus operator to concatenate containers; "
	//		<< "do not use [ ..., [nesting], ... ]." << endl;
	//}

	if (SynSym *clone = s.clone()) {
		nested = nested || s.isA(ContainerSym::TheType);
		theItems.append(clone);
		if (p >= 0) {
			// fill the gap with default probs as needed
			theProbs.stretch(theItems.count());
			while (theProbs.count() < theItems.count())
				theProbs.append(-1);
			theProbs.last() = p;
		}
		return true;
	}

	return false;
}

void ArraySym::reserve(int minCount) {
	theItems.stretch(minCount);
}

// unconditional add
void ArraySym::add(const SynSym &s, double p) {
	if (!cadd(s, p)) {
		Assert(theItemType); // cadd cannot fail otherwise
		cerr << s.loc() << "cannot add `" << s.type() << "' item to `"
			<< theItemType <<  "[]' array" << endl << xexit;
	}
}

ExpressionSym *ArraySym::bnOper(const Oper &op, const SynSym &exp) const {
	if (!op.plus())
		return ContainerSym::bnOper(op, exp);

	ArraySym *res = (ArraySym*)clone();
	Assert(res);
	if (exp.isA(TheType))
		res->append((const ArraySym&)exp.cast(TheType));
	else
		res->add(exp);
	return res;
}

const SynSym *ArraySym::itemProb(int idx, double &prob) const {
	int offset = 0;
	for (int i = 0; i < theItems.count(); ++i) {
		const int cnt = itemCountAt(i);
		if (offset <= idx && idx < offset + cnt) {
			const int iOffset = idx - offset;
			double iProb = -1;
			const SynSym *s = itemProbAt(i, iOffset, iProb);
			Assert(iProb >= 0);

			double assignedProb = -1;
			if (i < theProbs.count())
				assignedProb = theProbs[i];

			prob = iProb * actualProb(assignedProb);
			return s;
		}
		offset += cnt;
	}

	Assert(false);
	return 0;
}

int ArraySym::itemCountAt(int idx) const {
	if (nested && theItems[idx]->isA(ContainerSym::TheType)) {
		return ((const ContainerSym&)theItems[idx]->cast(ContainerSym::TheType)).count();
	} else {
		return 1;
	}
}

bool ArraySym::itemProbsSetAt(int idx) const {
	if (nested && theItems[idx]->isA(ContainerSym::TheType))
		return ((const ContainerSym&)theItems[idx]->cast(ContainerSym::TheType)).probsSet();
	else
		return false;
}

const SynSym *ArraySym::itemProbAt(int idx, int offset, double &prob) const {
	if (nested && theItems[idx]->isA(ContainerSym::TheType))
		return ((const ContainerSym&)theItems[idx]->cast(ContainerSym::TheType)).itemProb(offset, prob);
	prob = 1;
	return theItems[idx];
}

void ArraySym::copyProbs(Array<double> &res) const {
	const int cnt = count();
	res.stretch(cnt);
	for (int i = 0; i < cnt; ++i)
		res.append(prob(i));
}

bool ArraySym::probsSet() const {
	if (theProbs.count())
		return true;
	for (int i = 0; i < theItems.count(); ++i) {
		if (itemProbsSetAt(i))
			return true;
	}
	return false;
}

ostream &ArraySym::print(ostream &os, const String &pfx) const {
	os << '[';
	for (int i = 0; i < theItems.count(); ++i) {
		if (i)
			os << ", ";
		theItems[i]->print(os, pfx);

		const double p = explProb(i);
		if (p >= 0)
			os << ": " << (100*p) << '%';
	}
	os << ']';
	return os;
}

// uses a table distribution to model item probabilities
// selector is a distribution of item positions
// XXX: if no probs specified, we use uniform distribution to save memory
//      ideally, table distr should be smart enough to detect this case
RndDistr *ArraySym::makeSelector(const String &name) {
	if (!probsSet()) {
		RndGen *rng = new RndGen(GlbPermut(count(), rndArraySymSelector));
		return new UnifDistr(rng, 0, count());
	}

	Array<double> probs;
	copyProbs(probs);
	return TblDistr::FromDistrTable(name, probs);
}

// same as ContainerSym::forEach but probably faster because it
// does not use item()
void ArraySym::forEach(Visitor &v, RndGen *const rng) const {
	for (int i = 0; i < theItems.count(); ++i) {
		if (rng) {
			const double p = explProb(i);
			if (p >= 0 && !rng->event(p))
				continue;
		}

		const SynSym *s = theItems[i];
		if (nested && s->isA(ContainerSym::TheType)) {
			const ContainerSym &c =
				(const ContainerSym&)s->cast(ContainerSym::TheType);
			c.forEach(v, rng);
		} else {
			v.visit(*s);
		}
	}
}

bool ArraySym::GetNestedArraysFromItem(Array<const ArraySym*> &arrays, const SynSym *item) {
	if (item->isA(ClonerSym::TheType)) {
		const ClonerSym &c = (const ClonerSym&)item->cast(ClonerSym::TheType);
		for (int i = 0; i < c.cloneFactor(); ++i) {
			if (!GetNestedArraysFromItem(arrays, c.cloneSource()))
				return false;
		}
		return true; // even if added nothing
	}

	if (item->isA(ArraySym::TheType)) {
		int nestedContainers = 0;
		const ArraySym &array = (const ArraySym&)item->cast(ArraySym::TheType);

		for (int i = 0; i < array.theItems.count(); ++i) {
			if (array.theItems[i]->isA(ArraySym::TheType) || array.theItems[i]->isA(ClonerSym::TheType)) {
				if (!GetNestedArraysFromItem(arrays, array.theItems[i]))
					return false;
				++nestedContainers;
			}
		}

		if (!nestedContainers) {
			// deepest level: no containers inside the item
			arrays.append(&array);
			return true;
		}

		// check that all array elements were appropriate containers
		return nestedContainers == array.theItems.count();
	}

	return false; // item of a wrong type
}
