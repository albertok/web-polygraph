
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include <ctype.h>

#include "xstd/gadgets.h"
#include "pgl/PglStrRange.h"
#include "pgl/PglStrRangeLexer.h"


/* PglStrRangeLexer::Lexem */

PglStrRangeLexer::Lexem::Lexem(): theStart(0), theStop(0) {
}

PglStrRangeLexer::Lexem::Lexem(const char *aStart, const char *aStop):
	theStart(aStart), theStop(aStop) {
}


/* PglStrRangeLexer */

PglStrRangeLexer::PglStrRangeLexer(PglStrRange *anOwner, const String &anImage):
	theLexems(anImage.len()), theOwner(anOwner), theImage(anImage) {
	Assert(theOwner && theImage);
}

bool PglStrRangeLexer::parse() {
	// split into lexems (numbers, words, or special chars)
	for (const char *p = theImage.cstr(); *p;) {
		const char *start = p;
		const char ch = *start;
		if (isDigit(ch)) {
			do { ++p; } while (isDigit(*p));
		} else
		if (isalpha(ch)) {
			do { ++p; } while (isalpha(*p));
		} else {
			++p;
		}
		theLexems.enqueue(Lexem(start, p));
	}
	return true;
}

char PglStrRangeLexer::charAt(int pos) const {
	if (theLexems.count() <= pos)
		return (char)0;

	return theLexems.top(pos).tag();
}

void PglStrRangeLexer::step() {
	Assert(theLexems.count());
	Lexem &lm = theLexems.top();
	theOwner->addRangePoint(String(lm.start(), lm.stop()-lm.start()));
	skip();
}

void PglStrRangeLexer::skip(int count) {
	while (count--)
		theLexems.dequeue();
}

int PglStrRangeLexer::intAt(int pos) const {
	Assert(pos < theLexems.count());
	int n = -1;
	Assert(isInt(theLexems.top(pos).start(), n, 0, theOwner->currentBase()));
	return n;
}

// int or int-int
bool PglStrRangeLexer::rangeItem(bool isolated) {
	if (!isDigit(charAt(0)))
		return false;
	const int start = intAt(0);

	if (charAt(+1) == '-') {

		if (!isDigit(charAt(+2)))
			return false;
		const int stop = intAt(+2) + 1;
		if (stop < start)
			return false;

		skip(3);
		theOwner->addRangeInterval(start, stop, isolated);
		return true;
	}

	skip(1);
	theOwner->addRangeInterval(start, start+1, isolated);
	return true;
}

// rangeItem, rangeItem, ...
bool PglStrRangeLexer::range(bool isolated) {
	bool result = false;
	while (rangeItem(isolated)) {
		result = true;
//		if (next() == ',')
//			skip();
	}
	return result;
}

bool PglStrRangeLexer::isDigit(char ch) {
	switch (theOwner->currentBase()) {
	case 10:
		return isdigit(ch);
	case 16:
		return isxdigit(ch);
	}
	return false;
}
