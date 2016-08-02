
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ObjId.h"
#include "base/RndPermut.h"
#include "runtime/HttpPrinter.h"
#include "runtime/httpText.h"
#include "runtime/IOBuf.h"
#include "csm/ContentCfg.h"
#include "csm/BodyIter.h"


BodyIter::BodyIter(): theContentCfg(0), theBuf(0), theContentHash(-1) {
}

void BodyIter::contentSize(Size aContentSize, Size aSuffixSize) {
	theContentSize = aContentSize;
	theSuffixSize = aSuffixSize;
}

Size BodyIter::contentSize() const {
	if (!theSuffixSize.known()) {
		theSuffixSize = Should(theContentCfg && theOid) ?
			theContentCfg->calcContentSuffixSize(theOid) : Size(0);
	}
	// assumes response size is always known
	if (!theContentSize.known())
		calcContentSize();
	return theContentSize;
}

Size BodyIter::fullEntitySize() const {
	return contentSize();
}

Size BodyIter::middleSizeLeft() const {
	if (!ShouldUs(theContentSize.known()))
		return theContentSize;

	if (!(ShouldUs(theSuffixSize <= theContentSize)))
		return 0;

	const Size middleGoal = theContentSize - theSuffixSize;
	if (theBuiltSize >= middleGoal)
		return 0;

	return middleGoal - theBuiltSize;
}

void BodyIter::calcContentSize() const {
	Should(!theContentSize.known());

	Assert(!theBuiltSize.known());
	BodyIter *const encoder = clone();
	WrBuf buf;
	encoder->start(&buf);
	while (*encoder) {
		if (!encoder->pour())
			Assert(false);
		buf.reset();
	}
	theContentSize = encoder->builtSize();
	encoder->putBack();
}

void BodyIter::start(WrBuf *aBuf) {
	Assert(theOid);
	Assert(aBuf);
	theBuf = aBuf;
	theRng.seed(GlbPermut(theContentHash, rndBodyIter));
	theBuiltSize = 0;
}

bool BodyIter::canPour() const {
	return theBuf && !theBuf->full() && !pouredAll();
}

bool BodyIter::pouredAll() const {
	return theContentSize.known() && theBuiltSize >= theContentSize;
}

// By default, pour() handles body prefix, middle, and suffix parts.
// BodyIterators that do not have those parts override this implementation.
bool BodyIter::pour() {
	if (theBuiltSize == 0)
		pourPrefix();
	if (!pourMiddle())
		return false;
	if (canPour() && !middleSizeLeft())
		pourSuffix();
	return true;
}

void BodyIter::pourPrefix() {
	if (Should(theContentCfg && theBuf))
		theBuiltSize += theContentCfg->pourContentPrefix(theOid, *theBuf);
}

void BodyIter::pourSuffix() {
	if (Should(theContentCfg && theBuf))
		theBuiltSize += theContentCfg->pourContentSuffix(theOid, *theBuf);
}

// kids must override either this method or pour() that calls this method
bool BodyIter::pourMiddle() {
	Assert(false);
	return false;
}

bool BodyIter::pourRandom(const Size upToSz) {
	const Size rndOff = IOBuf::RandomOffset(offSeed(), theBuiltSize);
	const RndBuf &rndBuf = theContentCfg->rndBuf();
	const Size poured = theBuf->appendRndUpTo(rndOff, upToSz, rndBuf);
	theBuiltSize += poured;
	return poured > 0;
}

void BodyIter::putHeaders(HttpPrinter &hp) const {
	if (hp.putHeader(hfpContLength)) {
	const Size clen = contentSize();
	Assert(clen.known());
	// Debug suffix addition via HTTP headers. TODO: Make this configurable.
	//if (theSuffixSize.known())
	// 	hp << "X-Suffix-Length: " << theSuffixSize.byte() << crlf;
	hp << clen.byte() << crlf;
	}

	if (theContentCfg->theMimeType &&
		hp.putHeader(hfpContType))
		hp << theContentCfg->theMimeType << crlf;
	if (theOid.gzipContent())
		hp.putHeader(hfGzipContentEncoding);
}

void BodyIter::putBack() {
	theContentCfg->putBodyIter(this);
	// do not put code here
	// body iterator must not be used after its returned to the content configuration
}
