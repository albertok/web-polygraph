
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xparser/xparser.h"

#include <stdio.h>
#include <stdlib.h>

#include "xparser/GLexer.h"

GLexer::GLexer(istream &aSource, const String &aFName):
	theSource(aSource), theFName(aFName) {

	if (!theSource)
		cerr << here << "cannot open `" << theFName << "'" << endl << xexit;
	
	_lookahead.loc = TokenLoc(aFName, 1);
	_lookahead.ch = '\n'; // fool nextCh() to set isFirstOnLine
	nextCh();
}

GLexer::~GLexer() {
}

// interrets the FSA table and performs state transitions
// and returns next token
LexToken GLexer::scan() {
again:
	int state = initialState;
	LexToken token(-1, _lookahead.loc);
	for (;;) {
	    const int next = transition(state, _lookahead.ch);
	    if (next < 0) { // next transition would lead to error state
			token.id(finalInfo[state].tokenValue);
			if (token.id() >= 0) {
				filterProc f = finalInfo[state].filter;
				if (f) 
					f(this, token);
				if (token.id() == 0)
					goto again; // predefined ignored token

				//cerr << here << "token: " << token.spell() << " (" << token.id() << ")" << endl;
				return token;
			} else {
				if (!token.spell()) {
					if (_lookahead.ch == (char)EOF) {
						//cerr << here << "token: EOF" << endl;
						return LexToken(1, _lookahead.loc); // predefined EOFTOKEN
					}
					token += _lookahead;
					nextCh();
				}
				//cerr << here << "token: ERR" << endl;
				return LexToken(0, _lookahead.loc); // predefined ERROR
			}
		}
	    token += _lookahead;
	    nextCh();
	    state = next;
	}

	abort();
	return LexToken(-1, _lookahead.loc);
}

void GLexer::nextCh() {
	_lookahead.isFirstOnLine = _lookahead.ch == '\n'; // last char

	char c = theSource.get();
	if (theSource && c == '\n')
		++_lookahead.loc;

	//cerr << here << "next char is `" << c << "' (" << (int)c << ")" << endl;

	_lookahead.ch = theSource ? c : (char)EOF;
}
