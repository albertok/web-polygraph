
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "xstd/gadgets.h"
#include "runtime/polyErrors.h"
#include "csm/XmlTagParser.h"

XmlTagParser::XmlTagParser(): theStr(0), theEos(0) {
}

bool XmlTagParser::parse(const char *s, const char *eos) {
	return parseTagname(s, eos) && parseRest();
}

bool XmlTagParser::parseTagname(const char *s, const char *eos) {
	theTokens.reset();
	theStr = s;
	theEos = eos;
	theError = Error();

	skipSpace();
	addToken();

	if (!theError) {
		if (!theTokens.count())
			theError = errEmptyTag;
		else
		if (theTokens[0].valLen)
			theError = errTagnameValue;
	}

	// here is the place to set Error::Last()
	return !theError;
}

bool XmlTagParser::parseRest() {
	Assert(theTokens.count() == 1 && !theError);

	do {
		skipSpace();
		addToken();
	} while (theStr < theEos && !theError);

	// here is the place to set Error::Last()
	return !theError;
}

void XmlTagParser::addToken() {
	Token token;

	// name
	token.nameBeg = theStr;
	while (theStr < theEos && *theStr != '=' && !isspace(*theStr))
		++theStr;
	token.nameLen = theStr - token.nameBeg;
	token.valLen = 0;

	skipSpace();

	// value, if any
	if (theStr < theEos && *theStr == '=') {
		++theStr; // skip '='
		skipSpace();
		if (theStr >= theEos) {
			theError = errTagAttrValueExpected;
			return;
		}
		if (*theStr == '"' || *theStr == '\'') {
			// quoted string
			const char del = *theStr;
			++theStr;
			token.valBeg = theStr;
			while (theStr < theEos && *theStr != del)
				++theStr;
			if (theStr < theEos && *theStr == del) {
				token.valLen = theStr - token.valBeg;
				++theStr;
			} else {
				theError = errTagAttrValueOpen;
				return;
			}
		} else {
			// unquoted string
			token.valBeg = theStr;
			while (theStr < theEos && !isspace(*theStr))
				++theStr;
			token.valLen = theStr - token.valBeg;
			if (theStr < theEos && isspace(*theStr))
				++theStr;
		}
	} else {
		token.valBeg = theStr;
		token.valLen = 0;
	}

	if (token.nameLen)
		theTokens.append(token);
}

void XmlTagParser::skipSpace() {
	while (theStr < theEos && isspace(*theStr))
		++theStr;
}
