
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "xstd/gadgets.h"
#include "csm/XmlParser.h"


XmlParser::XmlParser(): theStr(0), theEos(0) {
	theTail.type = Node::tpNone;
	theTail.imageLen = 0;
}

bool XmlParser::parse(const char *s, const char *eos) {
	theNodes.reset();
	theStr = s;
	theEos = eos;
	theTail.imageBeg = eos;

	while (theStr < theEos) {
		Node node;
		node.imageBeg = theStr; // remember starting position
		node.type = skipNode(); // advance and identify type, if possible
		node.imageLen = theStr - node.imageBeg;

		if (node.type == Node::tpNone) {
			Assert(theStr == theEos);
			theTail.imageBeg = node.imageBeg;
		} else {
			theNodes.append(node);
			theTail.imageBeg = theStr;
		}
	}

	theTail.imageLen = eos - theTail.imageBeg;
	return theTail.imageLen == 0;
}

XmlParser::Node::Type XmlParser::skipNode() {
	// make sure an incomplete "<" or "<!" buffer does not confuse us

	if (theStr >= theEos)
		return Node::tpNone;

	if (*theStr != '<')
		return skipText();

	++theStr; // skip '<'
	if (theStr >= theEos)
		return Node::tpNone; // need more lookahead space

	if (*theStr != '!')
		return skipTag();

	++theStr; // skip '!'
	if (theStr >= theEos)
		return Node::tpNone; // need more lookahead space

	if (*theStr != '-')
		return skipTag();

	++theStr; // skip first '-'
	if (theStr >= theEos)
		return Node::tpNone; // need more lookahead space

	if (*theStr != '-')
		return skipTag();

	++theStr; // skip second '-'
	return skipComment();
}

XmlParser::Node::Type XmlParser::skipText() {
	while (theStr < theEos && *theStr != '<')
		++theStr;
	return Node::tpText; // never fails
}

XmlParser::Node::Type XmlParser::skipTag() {
	while (theStr < theEos && *theStr != '>')
		++theStr;

	if (theStr < theEos) {
		++theStr;
		return Node::tpTag;
	}

	return Node::tpNone;
}

XmlParser::Node::Type XmlParser::skipComment() {
	while (theStr < theEos) {
		// make progress; if we see "--", investigate further
		if (*theStr++ == '-' && theStr < theEos && *theStr == '-') {
			const char *p = theStr + 1;

			// SGML allows spaces between "--" and ">"; skip them, if any
			while (p < theEos && isspace(*p))
				++p;

			// end-of-comment found if we see ">"
			if (p < theEos && *p++ == '>') {
				theStr = p;
				return Node::tpComment;
			}
		}
	}

	return Node::tpNone;
}
