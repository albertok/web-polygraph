
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xparser/xparser.h"

#include "xparser/LexToken.h"


/* LexTokenChar */

LexTokenChar::LexTokenChar(): ch(0), isFirstOnLine(false) {
}


/* LexToken */

void LexToken::reset() {
	theId = -1;
	theSpell = 0;
	isFirstOnLine = false;
}

void LexToken::addChar(char c) {
	if (theSpell.len() < 4096)
		theSpell += c;
	else
		cerr << theLoc << "input token is already " << theSpell.len() << " characters long" << endl << xexit;
}

LexToken &LexToken::operator +=(const LexTokenChar &c) {
	addChar(c.ch);
	isFirstOnLine = c.isFirstOnLine;
	return *this;
}

void LexToken::trim(char c) {
	// ltrim
	int i = 0;
	while (i < theSpell.len() && theSpell[i] == c) ++i;

	// rtrim
	int j = theSpell.len() - 1;
	while (j > 0 && theSpell[j-1] == c) --j;
	
	if (j > i)
		theSpell = theSpell(i, j);
	else
		theSpell = "";
}
