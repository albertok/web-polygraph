
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "xstd/PrefixIdentifier.h"
#include "xstd/gadgets.h"
#include "base/AnyToString.h"
#include "csm/ContentDbase.h"
#include "csm/cdbEntries.h"
#include "csm/XmlParser.h"
#include "csm/XmlTagParser.h"
#include "csm/cdbBuilders.h"


/* CdbBuilder */

int CdbBuilder::TheLinkCount = 0;

CdbBuilder::CdbBuilder(): theDb(0), theBufB(0), theBufE(0) {
}

CdbBuilder::~CdbBuilder() {
}

void CdbBuilder::db(ContentDbase *aDb) {
	Assert(!theDb && aDb);
	theDb = aDb;
}

void CdbBuilder::configure(const String &aFname, const char *aBufB, const char *aBufE) {
	theFname = aFname;
	theBufB = aBufB;
	theBufE = aBufE;
}


/* MarkupParser */

PrefixIdentifier *MarkupParser::TheReplIdentifier = 0;
Array<String*> MarkupParser::TheReplacements;

bool MarkupParser::parse() {
	static XmlParser parser;
	(void)parser.parse(theBufB, theBufE);

	for (int i = 0; i < parser.nodeCount(); ++i) {
		const XmlParser::Node &node = parser.node(i);
		const String image = String(node.imageBeg, node.imageLen);
		switch (node.type) {
			case XmlParser::Node::tpText: {
				CdbeText *e = new CdbeText;
				e->image(image);
				addEntry(e);
				break;
			}

			case XmlParser::Node::tpTag: {
				parseTag(image); // will call addEntry
				break;
			}

			case XmlParser::Node::tpComment: {
				CdbeComment *e = new CdbeComment;
				e->image(image);
				addEntry(e);
				break;
			}

			default:
				Assert(false);
		}
	}

	if (!parser.nodeCount())
		cerr << theFname << ": warning: no valid markup found" << endl;

	if (parser.tail().imageLen) {
		cerr << theFname << ": warning: ignoring markup leftovers starting at ";
		cerr.write(parser.tail().imageBeg, Min(parser.tail().imageLen, 40));
		cerr << endl;
	}

	return true;
}

void MarkupParser::addEntry(CdbEntry *e) {
	theDb->add(e);
}

void MarkupParser::parseBlob(const String &blobImage) {
	if (blobImage.len() > 0) {
		CdbeBlob *e = new CdbeBlob;
		e->image(blobImage);
		addEntry(e);
	}
}

void MarkupParser::parseTag(const String &tagImage) {
	const char *tagB = tagImage.cstr();
	const char *tagE = tagImage.cstr() + tagImage.len();

	static XmlTagParser parser;
	if (!parser.parse(tagB+1, tagE-1)) {
		parseBlob(tagImage);
		return;
	}

	if (!isalpha(*parser.tagname().nameBeg)) { // closing tag, comment, etc.
		parseBlob(tagImage);
		return;
	}

	String keyPfx;
	keyPfx.append(parser.tagname().nameBeg, parser.tagname().nameLen);
	keyPfx += '.';

	// replace src="url" with src="/pg/embed/..."
	String newImage;
	const char *lastCopy = tagB;
	for (int i = 0; i < parser.attrCount(); ++i) {
		XmlTagParser::Token attr = parser.attr(i);
		if (attr.valBeg && *attr.valBeg != '#') { // skip URLs pointing to self
			const String keySfx = String(attr.nameBeg, attr.nameLen);
			if (const String *replacement = AttrValReplacement(keyPfx, keySfx)) {
				parseBlob(tagImage(lastCopy - tagB, attr.valBeg - tagB));

				lastCopy = attr.valBeg + attr.valLen;

				CdbeLink *link = new CdbeLink;
				link->contentCategory = *replacement;
				link->origImage = tagImage(attr.valBeg - tagB, lastCopy - tagB);
				addEntry(link);

				TheLinkCount++;
			}
		}
	}
	parseBlob(tagImage(lastCopy - tagB, tagE - tagB));
}

int MarkupParser::RegReplacement(const String &key, const String &ctype) {
	Assert(TheReplIdentifier);
	const int id = TheReplIdentifier->add(key);
	TheReplacements.put(new String(ctype), id);
	return id;
}

const String *MarkupParser::AttrValReplacement(const String &keyPfx, const String &keySfx) {
	if (!TheReplIdentifier) {
		TheReplIdentifier = new PrefixIdentifier;

		const String embedHtml = "/pg/embed/html";
		const String embedImage = "/pg/embed/image";
		const String embedData = "/pg/embed/data";

		// specific rules
		RegReplacement("applet.archive", embedData);
		RegReplacement("frame.src", embedHtml);
		RegReplacement("iframe.src", embedHtml);
		RegReplacement("img.src", embedImage);
		RegReplacement("img.lowsrc", embedImage);
		RegReplacement("img.usemap", embedHtml);
		RegReplacement("input.src", embedImage);
		RegReplacement("input.usemap", embedHtml);
		RegReplacement("layer.src", embedHtml);
		RegReplacement("object.data", embedData);
		RegReplacement("script.src", embedHtml);
		RegReplacement("link.href", embedData);

		// more general catch-all rules for attributes
		RegReplacement("background", embedImage);
		RegReplacement("href", embedHtml);
		RegReplacement("src", embedData);
		RegReplacement("data", embedData);

		TheReplIdentifier->optimize();
	}

	const String key = keyPfx + keySfx;
	int idx = TheReplIdentifier->lookup(key);
	if (idx <= 0)
		idx = TheReplIdentifier->lookup(keySfx);

	if (idx > 0) {
		Assert(idx < TheReplacements.count());
		return TheReplacements[idx];
	}
	return 0;
}


/* LinkOnlyParser */

LinkOnlyParser::LinkOnlyParser(): thePage(0) {
}

bool LinkOnlyParser::parse() {
	thePage = new CdbePage;
	theImage = String();
	if (MarkupParser::parse()) {
		flush();
		theDb->add(thePage);
		return true;
	}
	return false;
}

void LinkOnlyParser::addEntry(CdbEntry *e) {
	if (e->type() == cdbeLink) {
		flush();
		thePage->add(e);
	} else {
		theImage += PrintToString(*e);
		delete e;
	}
}

void LinkOnlyParser::flush() {
	if (theImage.len() > 0) {
		CdbeBlob *e = new CdbeBlob;
		e->image(theImage);
		thePage->add(e);
		theImage = String();
	}
}

/* VerbatimParser */

bool VerbatimParser::parse() {
	if (theBufB < theBufE) {
		CdbeBlob *e = new CdbeBlob;
		e->image(String(theBufB, theBufE-theBufB));
		theDb->add(e);
		return true;
	}
	cerr << theFname << ": warning: empty verbatim file" << endl;
	return true;
}
