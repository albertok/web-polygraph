/*
 * %%nomodsWarning
 */

/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xparser/ParsSym.h"
#include "pgl/PglParser.h"

%%prelude

%%attrdef

%%code

PglParser::PglParser(Lexer *l) : GParser(l) {
	// static int _condTable[] = %%condTable;

	acceptState = %%acceptState;
	// condTable = _condTable;
	maxState = %%states;
}

PglParser::~PglParser() {
}

int PglParser::actionTable(int state, int sym) {
	static int _actionTable[%%states][%%symbols] = %%actionTable;
	return _actionTable[state][sym];
}

ReduceInfo *PglParser::reduceTable(int i) {
	static ReduceInfo _reduceTable[] = %%reduceTable;
	return(&_reduceTable[i]);
}

%%postlude
