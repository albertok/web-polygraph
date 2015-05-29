
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/iostream.h"
#include "xstd/h/string.h"

#include "xstd/Assert.h"
#include "xstd/gadgets.h"
#include "pgl/PglStrBlocks.h"


/* PglStrBlock */

int PglStrBlock::diffCount(const PglStrBlock &b) const {
	if (theType == sbtPoint) {
		if (b.type() == sbtPoint)
			return ((const PglStrPointBlock*)this)->
				countDiffs((const PglStrPointBlock&)b);
		else
			return 2; // "big" difference
	}

	if (theType == sbtRange) {
		if (b.type() == sbtRange)
			return ((const PglStrRangeBlock*)this)->
				countDiffs((const PglStrRangeBlock&)b);
		else
			return 2; // "big" difference
	}

	Assert(false);
	return 2;
}

void PglStrBlock::merge(PglStrBlock &b) {
	if (type() == sbtPoint && b.type() == sbtPoint) {
		((PglStrPointBlock*)this)->
			mergeWith((const PglStrPointBlock&)b);
	} else
	if (type() == sbtRange && b.type() == sbtRange) {
		((PglStrRangeBlock*)this)->
			mergeWith((const PglStrRangeBlock&)b);
	} else {
		Assert(false);
	}
}


/* PglStrPointBlock */

PglStrPointBlock::PglStrPointBlock(const char *aStart, const char *aStop):
	PglStrBlock(sbtPoint), theStart(aStart), theStop(aStop) {
}

PglStrBlock *PglStrPointBlock::clone() const {
	return new PglStrPointBlock(theStart, theStop);
}

int PglStrPointBlock::count() const {
	return 1;
}

int PglStrPointBlock::countDiffs(const PglStrPointBlock &b) const {
	return
		(theStop-theStart == b.theStop-b.theStart &&
		strncmp(theStart, b.theStart, theStop-theStart) == 0) ? 0 : 2;
}

void PglStrPointBlock::mergeWith(const PglStrPointBlock &b) {
	Assert(countDiffs(b) == 0);
}

bool PglStrPointBlock::atLast() const {
	return true;
}

int PglStrPointBlock::pos() const {
	return 0;
}

void PglStrPointBlock::start() {
}

void PglStrPointBlock::next() {
	Assert(false);
}

void PglStrPointBlock::pos(int aPos) {
	Assert(aPos == 0);
}

void PglStrPointBlock::print(ostream &os) const {
	os.write(theStart, theStop-theStart);
}

void PglStrPointBlock::printCur(ostream &os) const {
	print(os);
}


/* PglStrRangeBlock */

PglStrRangeBlock::PglStrRangeBlock(int aStart, int aStop, bool beIsolated):
	PglStrBlock(sbtRange), theStart(aStart), theStop(aStop), thePos(aStart) ,
	isIsolated(beIsolated) {
}

PglStrBlock *PglStrRangeBlock::clone() const {
	return new PglStrRangeBlock(theStart, theStop, isIsolated);
}

int PglStrRangeBlock::count() const {
	return theStop - theStart;
}

int PglStrRangeBlock::countDiffs(const PglStrRangeBlock &b) const {
	// exact match
	if (theStart == b.theStart && theStop == b.theStop)
		return 0;

	// can only merge without changing the number of addresses
	// if one range follows the other
	if (theStop == b.theStart || b.theStop == theStart)
		return 1;

	// "big" difference
	return 2;
}

void PglStrRangeBlock::mergeWith(const PglStrRangeBlock &b) {
	theStart = Min(theStart, b.theStart);
	theStop = Max(theStop, b.theStop);
	// isIsolated does not change?
}

bool PglStrRangeBlock::atLast() const {
	return thePos == theStop-1;
}

int PglStrRangeBlock::pos() const {
	return thePos - theStart;
}

void PglStrRangeBlock::start() {
	thePos = theStart;
}

void PglStrRangeBlock::next() {
	thePos++;
}

void PglStrRangeBlock::pos(int aPos) {
	thePos = theStart + aPos; // local coordinates
	Assert(theStart <= thePos && thePos < theStop);
}

void PglStrRangeBlock::print(ostream &os) const {
	if (!isIsolated)
		os << '[';
	os << theStart;
	if (theStart != theStop-1)
		os << '-' << (theStop-1);
	if (!isIsolated)
		os << ']';
}

void PglStrRangeBlock::printCur(ostream &os) const {
	os << thePos;
}
