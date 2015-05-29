
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XPARSER_LEXTOKEN_H
#define POLYGRAPH__XPARSER_LEXTOKEN_H

#include "xstd/h/iosfwd.h"
#include "xstd/SrcLoc.h"


// Lex token used by parsers and toke components


// token location in the source stream
typedef SrcLoc TokenLoc;


struct LexTokenChar {
	TokenLoc loc;
	char ch;
	bool isFirstOnLine;

	LexTokenChar();
};

class LexToken {
	public:
		LexToken() { reset(); }
		LexToken(int anId, const TokenLoc &aLoc) { reset(); id(anId); loc(aLoc); }

		void reset();

		const TokenLoc &loc() const { return theLoc; }
		int id() const { return theId; }
		const String &spell() const { return theSpell; }
		bool firstOnLine() const { return isFirstOnLine; }
		
		LexToken &operator +=(const LexTokenChar &c);

		void id(int anId) { theId = anId; }
		void loc(const TokenLoc &aLoc) { theLoc = aLoc; }
		void firstOnLine(bool be) { isFirstOnLine = be; }

		void trim(char c);

	protected:
		void addChar(char c);

	protected:
		TokenLoc theLoc;
		int theId;
		String theSpell;
		bool isFirstOnLine;
};

#endif
