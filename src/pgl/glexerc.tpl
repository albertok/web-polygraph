/*
 * %%nomodsWarning
 */

/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglParser.h" /* to get tokens from the parser */
#include "pgl/PglLexer.h"

%%filters

PglLexer::PglLexer(istream &aSource, const String &aFName): GLexer(aSource, aFName) {
	static FinalInfo _finalInfo[] = %%finalStates;
	finalInfo = _finalInfo;
	initialState = %%initialState;
}

int PglLexer::transition(int s, int c) {
	static short int const transTable[%%states][%%charSet] = %%stateTransition;

	return(transTable[s][c+1]);
}
