
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/StringArrayBlocks.h"
#include "base/StringArray.h"


StringArray::StringArray(): theCount(-1) {
}

StringArray::StringArray(const StringArray &r): theCount(-1) {
	append(r);
}

StringArray::~StringArray() {
	reset();
}

void StringArray::reset() {
	while(theBlocks.count()) delete theBlocks.pop();
	theCount = -1;
}

StringArray &StringArray::operator =(const StringArray &r) {
	reset();
	append(r);
	return *this;
}

void StringArray::append(const String &s) {
	absorb(new StringArrayPoint(s));
}

void StringArray::absorb(StringArrayBlock *b) {
	theBlocks.append(b);
	if (theCount >= 0)
		theCount += b->count();
}

void StringArray::append(const StringArray &r) {
	theBlocks.stretch(theBlocks.count() + r.theBlocks.count());
	for (int i = 0; i < r.theBlocks.count(); ++i)
		theBlocks.append(r.theBlocks[i]->clone());
	theCount = -1;
}

int StringArray::count() const {
	// use cached info if available
	if (theCount >= 0)
		return theCount;

	theCount = 0;
	for (int i = 0; i < theBlocks.count(); ++i)
		theCount += theBlocks[i]->count();
	return theCount;
}

bool StringArray::find(const Area &member, int &idx) const {
	int skippedCount = 0;
	for (int i = 0; i < theBlocks.count(); ++i) {
		int offset = -1;
		if (theBlocks[i]->find(member, offset)) {
			idx = skippedCount + offset;
			return true;
		}
		skippedCount += theBlocks[i]->count();
	}
	return false;
}

String StringArray::item(int idx) const {
	int skippedCount = 0;
	for (int i = 0; i < theBlocks.count(); ++i) {
		const int nextOffset = skippedCount + theBlocks[i]->count();
		if (nextOffset > idx)
			return theBlocks[i]->item(idx - skippedCount);
		skippedCount = nextOffset;
	}

	Should(false);
	return String();
}
