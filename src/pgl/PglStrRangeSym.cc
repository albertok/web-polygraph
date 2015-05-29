
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "base/StringArray.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglStrRange.h"
#include "pgl/PglStrRangeSym.h"



const String StrRangeSym::TheType = "StrRange";

static String strStringArr = "string[]";


StrRangeSym::StrRangeSym(): StringArrayBlockSym(TheType),
	theRange(0), theStr(0) {
}

StrRangeSym::~StrRangeSym() {
	delete theRange;
	delete theStr;
}

bool StrRangeSym::isA(const String &type) const {
	return type == TheType || StringArrayBlockSym::isA(type);
}

SynSym *StrRangeSym::dupe(const String &type) const {
	if (isA(type)) {
		StrRangeSym *clone = new StrRangeSym;
		if (theRange)
			clone->range(new PglStrRange(*theRange));
		return clone;
	} else
	if (type == strStringArr) {
		ArraySym *arr = new ArraySym(StringSym::TheType);
		arr->add(*this);
		return arr;
	} else {
		return 0;
	}
}

int StrRangeSym::count() const {
	Assert(theRange);
	return theRange->count();
}

bool StrRangeSym::probsSet() const {
	return false; // we do not support probabilities for string ranges yet
}

const SynSym *StrRangeSym::itemProb(int idx, double &prob) const {
	Assert(theRange);

	delete theStr; 
	theStr = 0;

	String s;
	theRange->strAt(idx, s);
	theStr = new StringSym(s);

	const int cnt = theRange->count();
	Assert(cnt > 0);
	prob = 1.0/cnt;
	return theStr;
}

const StringArrayBlock &StrRangeSym::val() const {
	return *theRange;
}

ostream &StrRangeSym::print(ostream &os, const String &) const {
	Assert(theRange);
	os << '"';
	theRange->print(os);
	os << '"';
	return os;
}
