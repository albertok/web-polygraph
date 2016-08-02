
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"

#include "xstd/gadgets.h"
#include "base/StringArrayBlocks.h"


/* StringArrayBlock */

bool StringArrayBlock::canMerge(const StringArrayBlock &b) const {
	if (this->type() == b.type())
		return canMergeSameType(b);
	else
		return false;
}

void StringArrayBlock::merge(const StringArrayBlock &b) {
	if (this->type() == b.type())
		mergeSameType(b);
	else
		Should(false);
}


/* StringArrayPoint */

StringArrayBlock::TypeAnchor StringArrayPoint::TheTypeAnchor;

StringArrayPoint::StringArrayPoint(const String &aPoint):
	StringArrayBlock(&TheTypeAnchor), thePoint(aPoint) {
}

StringArrayBlock *StringArrayPoint::clone() const {
	return new StringArrayPoint(thePoint);
}

int StringArrayPoint::count() const {
	return 1;
}

bool StringArrayPoint::find(const Area &member, int &) const {
	return thePoint.len() == member.size() && thePoint.find(member) == 0;
}

bool StringArrayPoint::canMergeSameType(const StringArrayBlock &b) const {
	return thePoint == ((const StringArrayPoint&)b).thePoint;
}

void StringArrayPoint::mergeSameType(const StringArrayBlock &b) {
	Should(canMergeSameType(b));
}

String StringArrayPoint::item(int idx) const {
	if (Should(idx == 0))
		return thePoint;
	return String();
}


