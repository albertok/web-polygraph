/*
 * %%nomodsWarning
 */

/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLPARSER_H
#define POLYGRAPH__PGL_PGLPARSER_H

#include "xparser/GParser.h"

class PglParser : public GParser {
	public:
		PglParser(Lexer *l);
		virtual ~PglParser();

	protected:
		virtual int actionTable(int, int);
		virtual ReduceInfo *reduceTable(int);
};

// these are needed by lexer, argh!
%%tokens

#endif
