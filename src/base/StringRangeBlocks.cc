
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include <iomanip>
#include "xstd/h/iostream.h"

#include "xstd/gadgets.h"
#include "base/StringRangeBlocks.h"


/* StringRangeBlock */

int StringRangeBlock::diffCount(const StringRangeBlock &b) const {
	if (theType == sbtPoint) {
		if (b.type() == sbtPoint)
			return ((const StringRangePoint*)this)->
				countDiffs((const StringRangePoint&)b);
		else
			return 2; // "big" difference
	}

	if (theType == sbtInterval) {
		if (b.type() == sbtInterval)
			return ((const StringRangeInterval*)this)->
				countDiffs((const StringRangeInterval&)b);
		else
			return 2; // "big" difference
	}

	Assert(false);
	return 2;
}

void StringRangeBlock::merge(StringRangeBlock &b) {
	if (type() == sbtPoint && b.type() == sbtPoint) {
		((StringRangePoint*)this)->
			mergeWith((const StringRangePoint&)b);
	} else
	if (type() == sbtInterval && b.type() == sbtInterval) {
		((StringRangeInterval*)this)->
			mergeWith((const StringRangeInterval&)b);
	} else {
		Assert(false);
	}
}


/* StringRangePoint */

StringRangePoint::StringRangePoint(const String &aPoint):
	StringRangeBlock(sbtPoint), thePoint(aPoint) {
}

StringRangeBlock *StringRangePoint::clone() const {
	return new StringRangePoint(thePoint);
}

int StringRangePoint::count() const {
	return 1;
}

bool StringRangePoint::findTail(const Area &a, int &tailPos, int &idx) const {
	int myPos = thePoint.len() -1;
	int aPos = a.size() - 1;
	bool found = false;
	while (myPos >= 0 && aPos >= 0 && thePoint[myPos] == a.data()[aPos]) {
		found = true;
		tailPos = aPos;
		--myPos;
		--aPos;
	}

	if (found)
		idx = 0;

	return found;
}

int StringRangePoint::countDiffs(const StringRangePoint &b) const {
	return thePoint == b.thePoint ? 0 : 2;
}

void StringRangePoint::mergeWith(const StringRangePoint &b) {
	Assert(countDiffs(b) == 0);
}

bool StringRangePoint::atLast() const {
	return true;
}

int StringRangePoint::pos() const {
	return 0;
}

void StringRangePoint::start() {
}

void StringRangePoint::next() {
	Assert(false);
}

void StringRangePoint::pos(int aPos) {
	Assert(aPos == 0);
}

void StringRangePoint::print(ostream &os) const {
	os << thePoint;
}

void StringRangePoint::printCur(ostream &os) const {
	print(os);
}


/* StringRangeInterval */

StringRangeInterval::StringRangeInterval(int aStart, int aStop, bool beIsolated, int aBase):
	StringRangeBlock(sbtInterval), theStart(aStart), theStop(aStop),
	theBase(aBase), thePos(aStart), isIsolated(beIsolated) {
	Should(theBase == 10 || theBase == 16);
}

StringRangeBlock *StringRangeInterval::clone() const {
	return new StringRangeInterval(theStart, theStop, isIsolated, theBase);
}

int StringRangeInterval::count() const {
	return theStop - theStart;
}

bool StringRangeInterval::findTail(const Area &a, int &tailPos, int &idx) const {
	// does the area end with a number?
	int aPos = a.size() - 1;
	int foundPos = -1;
	while (aPos >= 0 && isdigit(a.data()[aPos])) {
		foundPos = aPos;
		--aPos;
	}

	if (foundPos >= 0) {
		// does the number belong to our range?
		int num = -1;
		if (isInt(a.data() + foundPos, num) && theStart <= num && num < theStop) {
			tailPos = foundPos;
			idx = num - theStart;
			return true;
		}
	}
		
	return false;
}

int StringRangeInterval::countDiffs(const StringRangeInterval &b) const {
	// different bases cannot merge
	if (theBase != b.theBase)
		return 2;

	// exact match
	if (theStart == b.theStart && theStop == b.theStop)
		return 0;

	// can only merge without changing the number of members
	// if one range follows the other
	if (theStop == b.theStart || b.theStop == theStart)
		return 1;

	// "big" difference
	return 2;
}

void StringRangeInterval::mergeWith(const StringRangeInterval &b) {
	Should(theBase == b.theBase);
	theStart = Min(theStart, b.theStart);
	theStop = Max(theStop, b.theStop);
	// isIsolated does not change?
}

bool StringRangeInterval::atLast() const {
	return thePos == theStop-1;
}

int StringRangeInterval::pos() const {
	return thePos - theStart;
}

void StringRangeInterval::start() {
	thePos = theStart;
}

void StringRangeInterval::next() {
	thePos++;
}

void StringRangeInterval::pos(int aPos) {
	thePos = theStart + aPos; // local coordinates
	Assert(theStart <= thePos && thePos < theStop);
}

void StringRangeInterval::print(ostream &os) const {
	const ios_fmtflags savedFlags = os.flags();
	os << setbase(theBase);

	if (!isIsolated)
		os << '[';
	os << theStart;
	if (theStart != theStop-1)
		os << '-' << (theStop-1);
	if (!isIsolated)
		os << ']';

	os.flags(savedFlags);
}

void StringRangeInterval::printCur(ostream &os) const {
	const ios_fmtflags savedFlags = os.flags();
	os << setbase(theBase) << thePos;
	os.flags(savedFlags);
}
