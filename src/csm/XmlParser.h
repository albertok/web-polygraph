
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_XMLPARSER_H
#define POLYGRAPH__CSM_XMLPARSER_H

#include "xstd/Array.h"

// splits XML-like content into nodes
// a tag node then can be parsed with XmlTagParser if needed
class XmlParser {
	public:
		struct Node {
			const char *imageBeg;
			int imageLen;
			enum Type { tpNone, tpText, tpTag, tpComment } type;

			bool text() const { return type == tpText; }
			bool tag() const { return type == tpTag; }
			bool comment() const { return type == tpComment; }
		};

	public:
		XmlParser();

		bool parse(const char *s, const char *eos);

		int nodeCount() const { return theNodes.count(); }
		const Node &node(int idx) const { return theNodes[idx]; }
		const Node &tail() const { return theTail; }

	protected:
		Node::Type skipNode();
		Node::Type skipText();
		Node::Type skipTag();
		Node::Type skipComment();

	protected:
		Array<Node> theNodes;
		const char *theStr;
		const char *theEos;
		Node theTail;
};

#endif
