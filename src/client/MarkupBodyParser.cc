
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "base/StatIntvlRec.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "csm/XmlParser.h"
#include "csm/XmlTagParser.h"
#include "csm/XmlTagIdentifier.h"
#include "client/CltXact.h"
#include "client/CltOpts.h"
#include "client/CltCfg.h"
#include "client/ParseBuffer.h"
#include "client/MarkupBodyParser.h"


BodyParserFarmT<MarkupBodyParser> MarkupBodyParser::TheParsers;


BodyParser *MarkupBodyParser::GetOne(CltXact *anOwner, const CltCfg *aCfg) {
	if (!TheParsers.capacity())
		TheParsers.limit(1024);

	MarkupBodyParser *parser = TheParsers.getTyped();
	parser->configure(anOwner, aCfg);
	return parser;
}

MarkupBodyParser::MarkupBodyParser(): theCfg(0) {
}

void MarkupBodyParser::reset() {
	resetSelf();
	BodyParser::reset();
}

void MarkupBodyParser::resetSelf() {
	theCfg = 0;
}

void MarkupBodyParser::configure(CltXact *anOwner, const CltCfg *aCfg) {
	BodyParser::configure(anOwner);
	Check(!theCfg && aCfg);
	theCfg = aCfg;
}

BodyParserFarm &MarkupBodyParser::farm() const {
	return TheParsers;
}

Size MarkupBodyParser::parse(const ParseBuffer &buf) {
	// do not search for tags if no tags need to be followed
	if (!theCfg->theContainerTags->tagCount()) {
		theOwner->noteContent(buf);
		return buf.size();
	}

	static XmlParser parser;
	(void)parser.parse(buf.data(), buf.data() + buf.size());

	for (int i = 0; i < parser.nodeCount(); ++i) {
		const XmlParser::Node &node = parser.node(i);
		if (node.tag())
			parseTag(node.imageBeg, node.imageBeg + node.imageLen);
	}

	const Size parsedSize = parser.tail().imageBeg - buf.data();
	theOwner->noteContent(buf.head(parsedSize));
	return parsedSize;
}

void MarkupBodyParser::noteLeftovers(const ParseBuffer &leftovers) {
	if (!TheCltOpts.ignoreBadContTags && ReportError(errContentLeftovers)) {
		dumpContext(Comment << "open markup tag near ", 
			leftovers.data(), leftovers.size()) << endc;
	}
	theOwner->noteContent(leftovers);
}

void MarkupBodyParser::noteOverflow(const ParseBuffer &buf) {
	if (!TheCltOpts.ignoreBadContTags && ReportError(errHugeContentToken)) {
		dumpContext(Comment << "huge markup tag near ",
			buf.data(), buf.size()) << endc;
	}
	theOwner->noteContent(buf);
}

void MarkupBodyParser::parseTag(const char *tagB, const char *tagE) {
	TheEmbedStats.tagSeen++;
	// TheEmbedStats.scriptSeen++;

	// static to avoid allocating array-of-tokens all the time
	static XmlTagParser tagParser;

	Error err;
	int tagIdx = -1;
	const XmlTagIdentifier &map = *theCfg->theContainerTags;
	if (tagParser.parseTagname(tagB+1, tagE-1)) {
		const XmlTagParser::Token &tagname = tagParser.tagname();
		if (!map.findTag(tagname.nameBeg, tagname.nameLen, tagIdx))
			return; // not an error
		tagParser.parseRest();
		TheEmbedStats.tagMatched++;
	} else {
		err = tagParser.error();
		Assert(err);
	}

	// iterate over attribute tokens within a tag
	for (int i = 0; !err && i < tagParser.attrCount(); ++i) {
		TheEmbedStats.attrSeen++;
		const XmlTagParser::Token &attr = tagParser.attr(i);
		if (!map.findAttr(tagIdx, attr.nameBeg, attr.nameLen))
			continue;

		const char *url = attr.valBeg;

		// ignore empty or local references
		if (attr.valLen <= 0 || *url == '#')
			continue;

		TheEmbedStats.attrMatched++;
		ReqHdr hdr;
		if (hdr.parseUri(url, url + attr.valLen, hdr.theUri))
			err = theOwner->noteEmbedded(hdr);
		else
			err = errForeignTag;
	}

	if (err && !TheCltOpts.ignoreBadContTags && ReportError(err))
		dumpContext(Comment << "offending markup tag: ", tagB, tagE - tagB) << endc;
}
