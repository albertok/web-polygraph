
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "runtime/HttpFindCrlf.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "client/CltXact.h"
#include "client/MultiPartParser.h"


BodyParserFarmT<MultiPartParser> MultiPartParser::TheParsers;
MultiPartParser::Step MultiPartParser::psMessageBeg = &MultiPartParser::parseMessageBeg;
MultiPartParser::Step MultiPartParser::psBoundary = &MultiPartParser::parseBoundary;
MultiPartParser::Step MultiPartParser::psPartHdrs = &MultiPartParser::parsePartHdrs;
MultiPartParser::Step MultiPartParser::psPartBody = &MultiPartParser::parsePartBody;
MultiPartParser::Step MultiPartParser::psPartEnd = &MultiPartParser::parsePartEnd;
MultiPartParser::Step MultiPartParser::psMessageEnd = &MultiPartParser::parseMessageEnd;


BodyParser *MultiPartParser::GetOne(CltXact *anOwner, BodyParser *aNextParser, const String &aBoundary) {
	if (!TheParsers.capacity())
		TheParsers.limit(1024);

	MultiPartParser *parser = TheParsers.getTyped();
	parser->configure(anOwner, aNextParser, aBoundary);
	return parser;
}


MultiPartParser::MultiPartParser(): theNextParser(0) {
	MultiPartParser::resetSelf();
}

MultiPartParser::~MultiPartParser() {
	MultiPartParser::resetSelf();
}

void MultiPartParser::reset() {
	resetSelf();
	BodyParser::reset();
}

void MultiPartParser::resetSelf() {
	if (theNextParser) {
		theNextParser->farm().put(theNextParser);
		theNextParser = 0;
	}

	theStep = psMessageBeg;
	theLeftBodySize = -1;
	needMoreData = false;
	theBoundary = String();
	theRepHdr.reset();
}

BodyParserFarm &MultiPartParser::farm() const {
	return TheParsers;
}

void MultiPartParser::configure(CltXact *anOwner, BodyParser *aNextParser, const String &aBoundary) {
	BodyParser::configure(anOwner);
	Check(!theNextParser ^ !aNextParser);
	theNextParser = aNextParser;
	Should(aBoundary.len() > 0);
	theBoundary = aBoundary;
}

Size MultiPartParser::parse(const ParseBuffer &buf) {
//clog << here << (int)buf.size() << " == 0x" << hex << (int)buf.size() << dec << " bytes to parse" << endl;
	theBuf = buf;
	needMoreData = theBuf.size() == 0;
	while (mayContinue()) {
		(this->*theStep)();
	}
//clog << here << "parsed: " << (int)(buf.size() - theBuf.size()) << " bytes" << endl;
	return buf.size() - theBuf.size();
}

void MultiPartParser::noteLeftovers(const ParseBuffer &leftovers) {
	if (theStep == psPartBody) {
		theNextParser->noteLeftovers(leftovers);
	} else {
		noteError(errContentLeftovers);
	}
}

void MultiPartParser::noteOverflow(const ParseBuffer &buf) {
	if (theStep == psPartBody) {
		theNextParser->noteOverflow(buf);
	} else {
		noteError(errMultipartHugeToken);
	}
}

bool MultiPartParser::mayContinue() const {
	return !needMoreData && !theError && theStep != psMessageEnd;
}

void MultiPartParser::noteError(const Error &e) {
	theError = true;
	if (ReportError(e)) {
		dumpContext(Comment << "in parsing context near ",
			theBuf.data(), theBuf.size()) << endc;
	}
}

void MultiPartParser::parseMessageBeg() {
	Size crlfBeg;
	Size crlfEnd;
	if (HttpFindCrlf(theBuf.data(), theBuf.size(), crlfBeg, crlfEnd)) {
		if (crlfBeg == 0)
			theBuf.consume(crlfEnd);
		else
			theStep = psBoundary;
		return;
	}

	needMoreData = true;
}

void MultiPartParser::parseBoundary() {
	Size crlfBeg;
	Size crlfEnd;
	if (HttpFindCrlf(theBuf.data(), theBuf.size(), crlfBeg, crlfEnd)) {
		const String data(theBuf.data(), crlfBeg);
		if (!data.startsWith("--" + theBoundary)) {
			noteError(errMultipartBoundary);
			return;
		}
		const String end(data(theBoundary.len() + 2, crlfBeg));
		if (end == "") {
			theStep = psPartHdrs;
			theBuf.consume(crlfBeg);
		} else
		if (end == "--") {
			theStep = psMessageEnd;
			theBuf.consume(crlfEnd);
		} else {
			noteError(errMultipartBoundary);
			return;
		}                  
		return;
	}

	needMoreData = true;
}

void MultiPartParser::parsePartHdrs() {
	theRepHdr.reset();
	if (theRepHdr.parse(theBuf.data(), theBuf.size())) {
		Assert(theRepHdr.theContRangeFirstByte >= 0);
		Assert(theRepHdr.theContRangeLastByte >= theRepHdr.theContRangeFirstByte);
		theLeftBodySize = theRepHdr.theContRangeLastByte - theRepHdr.theContRangeFirstByte + Size(1);
		theBuf.consume(theRepHdr.theHdrSize);
		const Error err = theOwner->noteReplyPart(theRepHdr);
		if (err)
			noteError(err);
		theStep = psPartBody;
		return;
	}

	needMoreData = true;
}

void MultiPartParser::parsePartBody() {
	Should(theLeftBodySize > 0);

	const Size toParseSize = Min(theLeftBodySize, theBuf.size());
	const Size parsedSz = theNextParser->noteData(theBuf.head(toParseSize));
	if (!Should(parsedSz <= toParseSize)) {
		noteError(errMultipartNextParser);
		return;
	}

	theBuf.consume(parsedSz);
	theLeftBodySize -= parsedSz;
	if (theLeftBodySize == 0)
		theStep = psPartEnd;
	else
		needMoreData = true;
}

void MultiPartParser::parsePartEnd() {
	Should(theLeftBodySize == 0);

	Size crlfBeg = 0;
	Size crlfEnd = 0;
	if (HttpFindCrlf(theBuf.data(), theBuf.size(), crlfBeg, crlfEnd)) {
		if (crlfBeg != 0) {
			noteError(errMultipartSuffix);
			return;
		}
		theBuf.consume(crlfEnd);
		theStep = psBoundary;
		return;
	}

	needMoreData = true;
}

void MultiPartParser::parseMessageEnd() {
	Should(false); // termination step, should not be called
}
