
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/sstream.h"

#include "xstd/gadgets.h"
#include "base/StringRangeBlocks.h"
#include "base/StringRange.h"


StringArrayBlock::TypeAnchor StringRange::TheTypeAnchor;


StringRange::StringRange(): StringArrayBlock(&TheTypeAnchor), theCurrentBase(10) {
}

StringRange::StringRange(const StringRange &r): StringArrayBlock(&TheTypeAnchor), theCurrentBase(r.theCurrentBase) {
	append(r);
}

StringRange::~StringRange() {
	reset();
}

StringArrayBlock *StringRange::clone() const {
	return new StringRange(*this);
}

void StringRange::reset() {
	while(theBlocks.count()) delete theBlocks.pop();
	// the current base is currenlty not reset
}

StringRange &StringRange::operator =(const StringRange &r) {
	reset();
	append(r);
	return *this;
}

void StringRange::currentBase(int aBase) {
	Should(aBase == 10 || aBase == 16);
	theCurrentBase = aBase;
}

int StringRange::currentBase() const {
	Should(theCurrentBase == 10 || theCurrentBase == 16);
	return theCurrentBase;
}

void StringRange::append(const StringRange &r) {
	if (theBlocks.count())
		Should(theCurrentBase == r.theCurrentBase);
	else
		theCurrentBase = r.theCurrentBase;
	theBlocks.stretch(theBlocks.count() + r.theBlocks.count());
	for (int i = 0; i < r.theBlocks.count(); ++i)
		theBlocks.append(r.theBlocks[i]->clone());
}

int StringRange::count() const {
	int cnt = theBlocks.count() ? 1 : 0;
	for (int i = 0; i < theBlocks.count(); ++i)
		cnt *= theBlocks[i]->count();
	return cnt;
}

bool StringRange::find(const Area &member, int &idx) const {
	int pos = 0;
	int cnt = 1;
	int tailPos = member.size();
	for (int i = theBlocks.count()-1; i >= 0; --i) {
		int blockPos = -1;
		if (!theBlocks[i]->findTail(member.head(tailPos), tailPos, blockPos))
			return false;
		pos += blockPos*cnt;
		cnt *= theBlocks[i]->count();
	}
	idx = pos;
	return true;
}

int StringRange::iterPos() const {
	int pos = 0;
	int cnt = 1;
	for (int i = theBlocks.count()-1; i >= 0; --i) {
		pos += theBlocks[i]->pos()*cnt;
		cnt *= theBlocks[i]->count();
	}
	return pos;
}

void StringRange::iterate(Iter iter) const {
	startIter();
	do {
		String str;
		currentIter(str);
		iter(str);
	} while (nextIter());
}

String StringRange::toStr() const {
	ostringstream buf;
	print(buf);
	return Stream2String(buf);
}

void StringRange::toStrs(Array<String*> &strs) const {
	startIter();
	do {
		String str;
		currentIter(str);
		strs.append(new String(str));
	} while (nextIter());
}

void StringRange::strAt(int idx, String &str) const {
	startIter();
	skipIter(idx);
	currentIter(str);
}

String StringRange::item(int idx) const {
	String s;
	strAt(idx, s);
	return s;
}

void StringRange::startIter() const {
	for (int i = 0; i < theBlocks.count(); ++i)
		theBlocks[i]->start();
}

void StringRange::skipIter(int count) const {
	count -= iterPos();
	Assert(count >= 0);
	int right = 1;
	int check = 0;
	for (int i = theBlocks.count()-1; count > check && i >= 0; --i) {
		StringRangeBlock &b = *theBlocks[i];
		const int pos = (count/right) % b.count();
		b.pos(pos);
		check += pos*right;
		right *= b.count();
	}
	Assert(count == check);
}

bool StringRange::nextIter() const {
	return nextIter(theBlocks.count()-1);
}

bool StringRange::nextIter(int level) const {
	if (level < 0)
		return false;

	StringRangeBlock &b = *theBlocks[level];
	// delegate to next level if overflow
	if (b.atLast()) {
		b.start();
		return nextIter(level-1);
	}

	b.next();
	return true;
}

void StringRange::currentIter(String &str) const {
	ostringstream buf;
	for (int i = 0; i < theBlocks.count(); ++i) {
		theBlocks[i]->printCur(buf);
	}
	str = Stream2String(buf);
}

void StringRange::addRangePoint(const String &point) {
	theBlocks.append(new StringRangePoint(point));
}

void StringRange::addRangeInterval(int start, int stop, bool isolated) {
	theBlocks.append(new StringRangeInterval(start, stop, isolated, theCurrentBase));
}

bool StringRange::canMergeSameType(const StringArrayBlock &b) const {
	const StringRange &r = (const StringRange &)b;

	if (!theBlocks.count()) // brand new object
		return true;

	if (theBlocks.count() != r.theBlocks.count())
		return false;

	int diffCount = 0;
	for (int i = 0; diffCount <= 1 && i < theBlocks.count(); ++i) {
		const StringRangeBlock &b1 = *theBlocks[i];
		const StringRangeBlock &b2 = *r.theBlocks[i];
		diffCount += b1.diffCount(b2);
	}
	// can merge ranges that differ by one block only because 
	// the range notation cannot express other resulting sets
	return diffCount == 1;
}

void StringRange::mergeSameType(const StringArrayBlock &b) {
	const StringRange &r = (const StringRange &)b;
	if (theBlocks.count()) {
		Assert(theBlocks.count() == r.theBlocks.count());
		for (int i = 0; i < theBlocks.count(); ++i)
			theBlocks[i]->merge(*r.theBlocks[i]);
	} else {
		// brand new object
		append(r);
	}

}

int StringRange::intervalCount() const {
	int count = 0;
	for (int i = 0; i < theBlocks.count(); ++i) {
		if (theBlocks[i]->count() > 1)
			count++;
	}
	return count;
}

ostream &StringRange::print(ostream &os) const {
	for (int i = 0; i < theBlocks.count(); ++i)
		theBlocks[i]->print(os);
	return os;
}

