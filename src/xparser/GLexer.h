
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XPARSER_GLEXER_H
#define POLYGRAPH__XPARSER_GLEXER_H

#include "xstd/h/iosfwd.h"
#include "xparser/Lexer.h"

struct FinalInfo;

// The GLexer class is the base class from which generated lexer classes
// are derived from.

class GLexer: public Lexer {
	public:
		GLexer(istream &aSource, const String &aFname);
		virtual ~GLexer();

		const String &fname() const { return theFName; }
		istream &source() { return theSource; }
		const LexTokenChar &tokenChar() const { return _lookahead; }
		void nextCh();

	protected:
		virtual LexToken scan();

		virtual int transition(int, int) = 0;
	
	protected:
		istream &theSource;
		String theFName;

		int initialState;
		FinalInfo *finalInfo;

		LexTokenChar _lookahead;
};


// The FinalInfo class provides information about final states of a
// generated FSA.

typedef void (*filterProc)(GLexer *, LexToken &);
struct FinalInfo {
	int tokenValue;
	filterProc filter;
};

#endif
