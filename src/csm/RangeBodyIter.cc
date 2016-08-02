
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "csm/ContentCfg.h"
#include "runtime/HttpPrinter.h"
#include "runtime/httpText.h"
#include "runtime/IOBuf.h"

#include "csm/RangeBodyIter.h"

RangeBodyIter::RangeBodyIter(const RangeList &ranges, BodyIter *const aProducer):
	theRanges(ranges),
	theProducer(aProducer),
	putPartHeader(false),
	theCurrRange(theRanges.end()) {
	Assert(theProducer);
	theOid = theProducer->oid();
	theContentCfg = theProducer->contentCfg();
	theContentHash = theProducer->contentHash();
	theContentSize =  Size(); // unknown
}

void RangeBodyIter::start(WrBuf *aBuf) {
	BodyIter::start(aBuf);
	Assert(theProducer);
	theProducer->start(&theProducerBuf);
	theProducerOffset = 0;

	Assert(!theRanges.empty());
	theCurrRange = theRanges.begin();
	theCurrRangeOffset = theCurrRange->theFirstByte;
	putPartHeader = multiRange();
}

void RangeBodyIter::stop() {
	if (theProducer) {
		theProducer->putBack();
		theProducer = 0;
	}
}

bool RangeBodyIter::pour() {
	Assert(theProducer);

	if (theCurrRange != theRanges.end()) {
		if (putPartHeader) {
			putRangeHeaders();
			putPartHeader = false;
		}

		if (theCurrRangeOffset < theProducerOffset) {
			theProducerOffset = 0;
			theProducerBuf.reset();
			theProducer->start(&theProducerBuf);
		}

		if (pourRange(theCurrRangeOffset, theCurrRange->theLastByte)) {
			++theCurrRange;
			if (theCurrRange != theRanges.end())
				theCurrRangeOffset = theCurrRange->theFirstByte;
			putPartHeader = multiRange();
		}
	}
        else {
		if (putPartHeader) {
			putTerminator();
			putPartHeader = false;
		}
		theContentSize = theBuiltSize;
	}

	return true;
}

bool RangeBodyIter::pourRange(Size &first, const Size &last) {
	while (canPour()) {
		Assert(first <= last);
		if (theProducer->canPour())
			theProducer->pour();
		if (Should(!theProducerBuf.empty())) {
			if (first - theProducerOffset < theProducerBuf.contSize()) {
				const char *const dataOffset = theProducerBuf.content() + first - theProducerOffset;
				Size dataSize(theBuf->spaceSize());
				if (last - first + Size(1) < dataSize)
					dataSize = last - first + Size(1);
				if (theProducerBuf.space() - dataOffset < dataSize)
					dataSize = theProducerBuf.space() - dataOffset;
				theBuf->append(dataOffset, dataSize);
				theBuiltSize += dataSize;
				first += dataSize;
				if (last + Size(1) == first)
					return true;
			}
			else {
				theProducerOffset += theProducerBuf.contSize();
				theProducerBuf.consumedAll();
			}
		}
	}

	return false;
}

Size RangeBodyIter::fullEntitySize() const {
	Assert(theProducer);
	return theProducer->fullEntitySize();
}


void RangeBodyIter::putRangeHeaders() {
	ofixedstream os(theBuf->space(), theBuf->spaceSize());

	if (theCurrRange != theRanges.begin())
		os << crlf;

	os << textMultipartSep << textMultipartBoundary << crlf;

	if (theContentCfg->theMimeType)
		os << hfpContType << theContentCfg->theMimeType << crlf;
	if (theOid.gzipContent())
		os << hfGzipContentEncoding;

	os
		<< hfpContRange
		<< (int)theCurrRange->theFirstByte << '-' << (int)theCurrRange->theLastByte
		<< '/' << (int)fullEntitySize()
		<< crlf << crlf;

	const Size added(os.tellp());
	theBuf->appended(added);
	theBuiltSize += added;
}

void RangeBodyIter::putTerminator() {
	ofixedstream os(theBuf->space(), theBuf->spaceSize());

	os
		<< crlf
		<< textMultipartSep << textMultipartBoundary << textMultipartSep << crlf;

	const Size added(os.tellp());
	theBuf->appended(added);
	theBuiltSize += added;
}

void RangeBodyIter::putHeaders(HttpPrinter &hp) const {
	if (!multiRange()) {
		BodyIter::putHeaders(hp);
		if (hp.putHeader(hfpContRange)) {
			hp << (int)theRanges.front().theFirstByte
				<< '-' << (int)theRanges.front().theLastByte
				<< "/" << (int)fullEntitySize()
				<< crlf;
		}
	} else
		hp.putHeader(hfMultiRangeContType);
}

RangeBodyIter *RangeBodyIter::clone() const {
	RangeBodyIter *const i = new RangeBodyIter(theRanges, theProducer->clone());
	i->contentSize(theContentSize, theSuffixSize);
	return i;
}

bool RangeBodyIter::multiRange() const {
	Assert(!theRanges.empty());
	return ++theRanges.begin() != theRanges.end();
}
