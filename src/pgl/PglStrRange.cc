
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglStrRangeLexer.h"
#include "pgl/PglStrBlocks.h"
#include "pgl/PglStrRange.h"


ArraySym *PglStrRange::toSyms(const TokenLoc &loc) const {
	ArraySym *arr = new ArraySym(StringSym::TheType);

	startIter();
	do {
		String str;
		currentIter(str);
		StringSym ss(str);
		if (loc)
			ss.loc(loc);
		arr->add(ss); // XXX: use range!
	} while (nextIter());

	return arr;
}

bool PglStrRange::parse(const String &val) {
	PglStrRangeLexer lexs(this, val);

	// convert val into lexems
	if (!lexs.parse())
		return false;

	// convert lexems into blocks
	while (lexs) {
		if (escapeChar(lexs.next()) && lexs.next(1)) {
			lexs.skip();
			lexs.step();
		} else
		if (optRangeBeg(lexs.next())) {
			lexs.step(); // keep a separator
			(void)lexs.range(true); // optional
		} else
		if (reqRangeBeg(lexs.next())) {
			const char start = lexs.next();
			lexs.skip(); // skip the range separator
			if (lexs.range(false) && reqRangeEnd(start, lexs.next()))
				lexs.skip();
			else
				return false; // invalid range specs
		} else
		if (reqRangeEnd(lexs.next())) {
			return false; // unbalanced required range separator
		} else {
			lexs.step(); // simple label
		}
	}

	return true;
}

bool PglStrRange::escapeChar(char ch) const {
	return ch == '\\';
}

bool PglStrRange::optRangeBeg(char) const {
	return false;
}

bool PglStrRange::reqRangeBeg(char ch) const {
	return ch == '[';
}

bool PglStrRange::reqRangeEnd(char ch) const {
	return ch == ']';
}

bool PglStrRange::reqRangeEnd(char, char ch) const {
	return reqRangeEnd(ch);
}
