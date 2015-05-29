
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ObjId.h"
#include "base/RndPermut.h"
#include "runtime/httpText.h"
#include "runtime/IOBuf.h"
#include "csm/ContentCfg.h"
#include "csm/BodyIter.h"


BodyIter::BodyIter(): theContentCfg(0), theBuf(0), theContentHash(-1) {
}

void BodyIter::contentSize(Size aContentSize) {
	theContentSize = aContentSize;
}

Size BodyIter::contentSize() const {
	// assumes response size is always known
	if (!theContentSize.known())
		calcContentSize();
	return theContentSize;
}

Size BodyIter::fullEntitySize() const {
	return contentSize();
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

bool BodyIter::pour() {
	if (theBuiltSize == 0)
		pourPrefix();
	return pourBody();
}

void BodyIter::pourPrefix() {
	if (Should(theContentCfg && theBuf))
		theBuiltSize += theContentCfg->pourContentPrefix(theOid, *theBuf);
}

void BodyIter::putHeaders(ostream &os) const {
	const Size clen = contentSize();
	Assert(clen.known());
	os << hfpContLength << (int)clen << crlf;

	if (theContentCfg->theMimeType)
		os << hfpContType << theContentCfg->theMimeType << crlf;
	if (theOid.gzipContent())
		os << hfGzipContentEncoding;
}

void BodyIter::putBack() {
	theContentCfg->putBodyIter(this);
	// do not put code here
	// body iterator must not be used after its returned to the content configuration
}
