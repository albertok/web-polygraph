
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XPARSER_LEXER_H
#define POLYGRAPH__XPARSER_LEXER_H

#include "xparser/LexToken.h"

// generic lexer
class Lexer {
	public:
		virtual ~Lexer() {}

		const LexToken &nextToken() { return(theToken = scan()); }
		const LexToken &token() const { return theToken; }

		int symbol() const { return token().id(); };
		const String &spelling() const { return token().spell(); }

	protected:
		virtual LexToken scan() = 0;

	protected:
		LexToken theToken;
};

#endif
