
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/Assert.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglClonerSym.h"



String ClonerSym::TheType = "Cloner";

static String strContainer = "Container";


ClonerSym::ClonerSym(const SynSym &aSource, int aCount): ContainerSym(TheType),
	theSource(aSource.clone()), theCount(aCount) {
	Assert(theSource);
	Assert(theCount > 0);
}

ClonerSym::~ClonerSym() {
	delete theSource;
}

bool ClonerSym::isA(const String &type) const {
	if (type == TheType || ContainerSym::isA(type))
		return true;

	if (const char *p = type.str("[]"))
		return theSource->isA(type(0, p - type.cstr()));

	return false;
}

SynSym *ClonerSym::dupe(const String &type) const {
	if (isA(type)) {
		ClonerSym *clone = new ClonerSym(*theSource, theCount);
		return clone;
	}

	if (const char *p = type.str("[]")) {
		const String itemType = type(0, p - type.cstr());
		if (theSource->canBe(itemType)) {
			ArraySym *arr = new ArraySym(itemType);
			arr->add(*this);
			return arr;
		}
	}

	return 0;
}

int ClonerSym::count() const {
	if (theSource->isA(strContainer))
		return theCount*((const ContainerSym&)theSource->cast(strContainer)).count();
	else
		return theCount;
}

bool ClonerSym::probsSet() const {
	if (theSource->isA(strContainer))
		return ((const ContainerSym&)theSource->cast(strContainer)).probsSet();
	else
		return false;
}

const SynSym *ClonerSym::itemProb(int idx, double &prob) const {
	const int cnt = count();
	Assert(cnt > 0);
	Assert(0 <= idx && idx < cnt);
	if (theSource->isA(strContainer)) {
		const ContainerSym &c = (const ContainerSym&)theSource->cast(strContainer);
		const int iOffset = idx / theCount;
		const SynSym *i = c.itemProb(iOffset, prob);
		prob /= theCount;
		return i;
	} else {
		prob = 1.0/theCount;
		return theSource;
	}
}

void ClonerSym::forEach(Visitor &v, RndGen *const rng) const {
	if (theSource->isA(strContainer)) {
		const ContainerSym &c = (const ContainerSym&)theSource->cast(strContainer);
		for (int i = 0; i < theCount; ++i)
			c.forEach(v, rng);
	} else {
		for (int i = 0; i < theCount; ++i)
			v.visit(*theSource);
	}
}

ostream &ClonerSym::print(ostream &os, const String &pfx) const {
	return theSource->print(os, pfx) << " ** " << theCount;
}
