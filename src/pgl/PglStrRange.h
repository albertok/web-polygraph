
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSTRRANGE_H
#define POLYGRAPH__PGL_PGLSTRRANGE_H

#include "base/StringRange.h"
#include "xparser/LexToken.h"

class String;
class ArraySym;
class PglStrBlock;
class StringArray;

// a string range (e.g., "blah[1-100].foo") parser and type converter
class PglStrRange: public StringRange {
	public:
		typedef void (*Iter)(const String &);
		friend class PglStrRangeLexer;

	public:
		virtual ArraySym *toSyms(const TokenLoc &loc) const;

		virtual bool parse(const String &val);

	protected:
		virtual bool escapeChar(char ch) const;
		virtual bool optRangeBeg(char ch) const;
		virtual bool reqRangeBeg(char ch) const;
		virtual bool reqRangeEnd(char ch) const;
		virtual bool reqRangeEnd(char beg, char end) const;
		
	protected:
};

#endif
