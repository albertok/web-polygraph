
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_XMLTAGPARSER_H
#define POLYGRAPH__CSM_XMLTAGPARSER_H

#include "xstd/Array.h"
#include "xstd/Error.h"

// splits <tagname attr=value attr=value> (no brackets) into tokens
class XmlTagParser {
	public:
		struct Token {
			const char *nameBeg;
			const char *valBeg;
			int nameLen;
			int valLen;
		};

	public:
		XmlTagParser();

		const Error &error() const { return theError; }

		// use either parse() or parseTagname(), parseRest() pair
		bool parse(const char *s, const char *eos);
		bool parseTagname(const char *s, const char *eos);
		bool parseRest();

		int attrCount() const { return theTokens.count() - 1; }
		const Token &tagname() const { return theTokens[0]; }
		const Token &attr(int idx) { return theTokens[idx+1]; }

	protected:
		void addToken();
		void skipSpace();

	protected:
		Array<Token> theTokens;
		const char *theStr;
		const char *theEos;
		Error theError;
};

#endif
