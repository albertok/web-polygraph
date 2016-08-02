
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/ZLib.h"

#include "base/RndPermut.h"
#include "runtime/LogComment.h"
#include "runtime/IOBuf.h"
#include "runtime/httpText.h"
#include "csm/ContentDbase.h"

#include "csm/ContentCfg.h"
#include "csm/InjectIter.h"
#include "csm/cdbEntries.h"
#include "csm/GzipEncoder.h"

#include "xstd/gadgets.h"


GzipEncoder::GzipEncoder(int aLevel, BodyIter *aProducer): 
	theLevel(aLevel), theProducer(aProducer), theEncoder(0) {
	Assert(theLevel >= 0);
	Assert(theProducer);
	theOid = theProducer->oid();
	theContentCfg = theProducer->contentCfg();
	theContentHash = theProducer->contentHash();
	theContentSize = Size(); // unknown
	theSuffixSize = 0; // none (but theProducer may have it)
}

GzipEncoder::~GzipEncoder() {
	delete theEncoder;
}

void GzipEncoder::start(WrBuf *aBuf) {
	BodyIter::start(aBuf);
	Assert(theProducer);
	theProducer->start(&theProducerBuf);

	delete theEncoder;
	theEncoder = new zlib::Deflator(theLevel);
}

void GzipEncoder::stop() {
	if (theProducer) {
		theProducer->putBack();
		theProducer = 0;
	}
}

bool GzipEncoder::pour() {
	while ((theProducer->canPour() || !theProducerBuf.empty() || theEncoder->needMoreSpace()) && this->canPour()) {
		if (theProducer->canPour())
			theProducer->pour();

		if (Should(!theProducerBuf.empty() || theEncoder->needMoreSpace())) {
			theEncoder->data(theProducerBuf.content(), theProducerBuf.contSize());
			theEncoder->space(theBuf->space(), theBuf->spaceSize());

			// compress
			Size dataDelta, spaceDelta;
			const bool donePouring = theProducer->pouredAll();
			const zlib::Stream::ZFlush flush = donePouring ?
				zlib::Stream::zFinish : zlib::Stream::zNoFlush;
			if (theEncoder->perform(spaceDelta, dataDelta, flush)) {
				theProducerBuf.consumed(dataDelta);
				theBuf->appended(spaceDelta);
				theBuiltSize += spaceDelta;
				if (!theContentSize.known() && donePouring && theProducerBuf.empty() && !theEncoder->needMoreSpace())
					theContentSize = theBuiltSize;
			} else {
				Comment << "error: a called to zlib::deflate failed: " <<
					theEncoder->error() << endc;
				return false;
			}
		}
	}
	return true;
}

GzipEncoder *GzipEncoder::clone() const {
	GzipEncoder *const i = new GzipEncoder(theLevel, theProducer->clone());
	i->contentSize(theContentSize, theSuffixSize);
	return i;
}
