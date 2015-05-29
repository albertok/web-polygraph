
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/SingleRangeSym.h"



const String SingleRangeSym::TheType = "SingleRange";

static const String strFirstByteAbsolute = "first_byte_pos_absolute";
static const String strFirstByteRelative = "first_byte_pos_relative";
static const String strLastByteAbsolute = "last_byte_pos_absolute";
static const String strLastByteRelative = "last_byte_pos_relative";
static const String strSuffixLengthAbsolute = "suffix_length_absolute";
static const String strSuffixLengthRelative = "suffix_length_relative";

SingleRangeSym::SingleRangeSym(): RangeSym(TheType, new PglRec) {
	theRec->bAdd(SizeSym::TheType, strFirstByteAbsolute, 0);
	theRec->bAdd(NumSym::TheType, strFirstByteRelative, 0);
	theRec->bAdd(SizeSym::TheType, strLastByteAbsolute, 0);
	theRec->bAdd(NumSym::TheType, strLastByteRelative, 0);
	theRec->bAdd(SizeSym::TheType, strSuffixLengthAbsolute, 0);
	theRec->bAdd(NumSym::TheType, strSuffixLengthRelative, 0);
}

SingleRangeSym::SingleRangeSym(const String &aType, PglRec *aRec): RangeSym(aType, aRec) {
}

bool SingleRangeSym::isA(const String &type) const {
	return RangeSym::isA(type) || type == TheType;
}

SynSym *SingleRangeSym::dupe(const String &type) const {
	if (isA(type))
		return new SingleRangeSym(this->type(), theRec->clone());
	return RangeSym::dupe(type);
}

bool SingleRangeSym::firstByteAbsolute(BigSize &sz) const {
	return getSize(strFirstByteAbsolute, sz);
}

bool SingleRangeSym::firstByteRelative(double &f) const {
	return getDouble(strFirstByteRelative, f);
}

bool SingleRangeSym::lastByteAbsolute(BigSize &sz) const {
	return getSize(strLastByteAbsolute, sz);
}

bool SingleRangeSym::lastByteRelative(double &f) const {
	return getDouble(strLastByteRelative, f);
}

bool SingleRangeSym::suffixLengthAbsolute(BigSize &sz) const {
	return getSize(strSuffixLengthAbsolute, sz);
}

bool SingleRangeSym::suffixLengthRelative(double &f) const {
	return getDouble(strSuffixLengthRelative, f);
}
