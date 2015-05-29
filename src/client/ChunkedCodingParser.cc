
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
#include "client/ChunkedCodingParser.h"


BodyParserFarmT<ChunkedCodingParser> ChunkedCodingParser::TheParsers;
ChunkedCodingParser::Step ChunkedCodingParser::psChunkBeg = &ChunkedCodingParser::parseChunkBeg;
ChunkedCodingParser::Step ChunkedCodingParser::psChunkBody = &ChunkedCodingParser::parseChunkBody;
ChunkedCodingParser::Step ChunkedCodingParser::psChunkEnd = &ChunkedCodingParser::parseChunkEnd;
ChunkedCodingParser::Step ChunkedCodingParser::psTrailer = &ChunkedCodingParser::parseTrailer;
ChunkedCodingParser::Step ChunkedCodingParser::psMessageEnd = &ChunkedCodingParser::parseMessageEnd;


BodyParser *ChunkedCodingParser::GetOne(CltXact *anOwner, BodyParser *aNextParser) {
	if (!TheParsers.capacity())
		TheParsers.limit(1024);

	ChunkedCodingParser *parser = TheParsers.getTyped();
	parser->configure(anOwner, aNextParser);
	return parser;
}


ChunkedCodingParser::ChunkedCodingParser(): theNextParser(0) {
	ChunkedCodingParser::resetSelf();
}

ChunkedCodingParser::~ChunkedCodingParser() {
	ChunkedCodingParser::resetSelf();
}

void ChunkedCodingParser::reset() {
	resetSelf();
	BodyParser::reset();
}

void ChunkedCodingParser::resetSelf() {
	if (theNextParser) {
		theNextParser->farm().put(theNextParser);
		theNextParser = 0;
	}

	theStep = psChunkBeg;
	theChunkSize = theLeftBodySize = -1;
	needMoreData = false;
	theError = false;
}

BodyParserFarm &ChunkedCodingParser::farm() const {
	return TheParsers;
}

void ChunkedCodingParser::configure(CltXact *anOwner, BodyParser *aNextParser) {
	BodyParser::configure(anOwner);
	Check(!theNextParser ^ !aNextParser);
	theNextParser = aNextParser;
}

Size ChunkedCodingParser::parse(const ParseBuffer &buf) {
//clog << here << (int)buf.size() << " == 0x" << hex << (int)buf.size() << dec << " bytes to parse" << endl;
	theBuf = buf;
	needMoreData = theBuf.size() == 0;
	while (mayContinue()) {
		(this->*theStep)();
	}
//clog << here << "parsed: " << (int)(buf.size() - theBuf.size()) << " bytes" << endl;
	return buf.size() - theBuf.size();
}

void ChunkedCodingParser::noteLeftovers(const ParseBuffer &leftovers) {
	if (theStep == psChunkBody) {
		theNextParser->noteLeftovers(leftovers);
	} else {
		noteError(errContentLeftovers);
	}
}

void ChunkedCodingParser::noteOverflow(const ParseBuffer &buf) {
	if (theStep == psChunkBody) {
		theNextParser->noteOverflow(buf);
	} else {
		noteError(errChunkHugeToken);
	}
}

bool ChunkedCodingParser::mayContinue() const {
	return !needMoreData && !theError && theStep != psMessageEnd;
}

void ChunkedCodingParser::noteError(const Error &e) {
	theError = true;
	if (ReportError(e)) {
		dumpContext(Comment << "in parsing context near ",
			theBuf.data(), theBuf.size()) << endc;
	}
}

void ChunkedCodingParser::parseChunkBeg() {
	Should(theChunkSize < 0);

	Size crlfBeg = 0;
	Size crlfEnd = 0;
	if (HttpFindCrlf(theBuf.data(), theBuf.size(), crlfBeg, crlfEnd)) {
		int size = -1;
		if (isInt(theBuf.data(), size, 0, 16)) {
//clog << here << "found chunk size: " << size << " == 0x" << hex << size << dec << endl;
			if (size < 0) {
				noteError(errChunkNegativeSize);
				return;
			}
			theBuf.consume(crlfEnd);
			theChunkSize = theLeftBodySize = size;
			theStep = theChunkSize == 0 ? psTrailer : psChunkBody;
			return;
		}
		noteError(errChunkSize);
		return;
	}

	needMoreData = true;
}

void ChunkedCodingParser::parseChunkBody() {
	Should(theLeftBodySize > 0);

	const Size toParseSize = Min(theLeftBodySize, theBuf.size());
	const Size parsedSz = theNextParser->noteData(theBuf.head(toParseSize));
	if (!Should(parsedSz <= toParseSize)) {
		noteError(errChunkNextParser);
		return;
	}

	theBuf.consume(parsedSz);
	theLeftBodySize -= parsedSz;
	if (theLeftBodySize == 0)
		theStep = psChunkEnd;
	else
		needMoreData = true;
}

void ChunkedCodingParser::parseChunkEnd() {
	Should(theLeftBodySize == 0);

	Size crlfBeg = 0;
	Size crlfEnd = 0;
	if (HttpFindCrlf(theBuf.data(), theBuf.size(), crlfBeg, crlfEnd)) {
		if (crlfBeg != 0) {
			noteError(errChunkSuffix);
			return;
		}
		theBuf.consume(crlfEnd);
		theChunkSize = -1; // done with the current chunk
		theStep = psChunkBeg;
		return;
	}
	
	needMoreData = true;
}

void ChunkedCodingParser::parseTrailer() {
	Should(theChunkSize == 0);

	while (mayContinue())
		parseTrailerHeader();
}

void ChunkedCodingParser::parseTrailerHeader() {
	Size crlfBeg = 0;
	Size crlfEnd = 0;
	if (HttpFindCrlf(theBuf.data(), theBuf.size(), crlfBeg, crlfEnd)) {
		if (crlfBeg > 0)
			theOwner->noteTrailerHeader(theBuf.head(crlfBeg));

		theBuf.consume(crlfEnd);

		if (crlfBeg == 0) {
			theStep = psMessageEnd;
			theOwner->noteEndOfTrailer();
		}

		return;
	}

	needMoreData = true;
}

void ChunkedCodingParser::parseMessageEnd() {
	Should(false); // termination step, should not be called
}

