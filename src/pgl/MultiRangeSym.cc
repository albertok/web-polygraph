
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/MultiRangeSym.h"



const String MultiRangeSym::TheType = "MultiRange";

static const String strFirstRangeStartAbsolute = "first_range_start_absolute";
static const String strFirstRangeStartRelative = "first_range_start_relative";
static const String strRangeLengthAbsolute = "range_length_absolute";
static const String strRangeLengthRelative = "range_length_relative";
static const String strRangeCount = "range_count";
static const String strInt_distr = "int_distr";
static const String strFloat_distr = "float_distr";
static const String strSize_distr = "size_distr";

MultiRangeSym::MultiRangeSym(): RangeSym(TheType, new PglRec) {
	theRec->bAdd(strSize_distr, strFirstRangeStartAbsolute, 0);
	theRec->bAdd(strFloat_distr, strFirstRangeStartRelative, 0);
	theRec->bAdd(strSize_distr, strRangeLengthAbsolute, 0);
	theRec->bAdd(strFloat_distr, strRangeLengthRelative, 0);
	theRec->bAdd(strInt_distr, strRangeCount, 0);
}

MultiRangeSym::MultiRangeSym(const String &aType, PglRec *aRec): RangeSym(aType, aRec) {
}

bool MultiRangeSym::isA(const String &type) const {
	return RangeSym::isA(type) || type == TheType;
}

SynSym *MultiRangeSym::dupe(const String &type) const {
	if (isA(type))
		return new MultiRangeSym(this->type(), theRec->clone());
	return RangeSym::dupe(type);
}

RndDistr *MultiRangeSym::first_range_start_absolute() const {
	return getDistr(strFirstRangeStartAbsolute);
}

RndDistr *MultiRangeSym::first_range_start_relative() const {
	return getDistr(strFirstRangeStartRelative);
}

RndDistr *MultiRangeSym::range_length_absolute() const {
	return getDistr(strRangeLengthAbsolute);
}

RndDistr *MultiRangeSym::range_length_relative() const {
	return getDistr(strRangeLengthRelative);
}

RndDistr *MultiRangeSym::range_count() const {
	return getDistr(strRangeCount);
}
