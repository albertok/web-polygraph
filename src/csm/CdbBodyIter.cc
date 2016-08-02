
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "base/RndPermut.h"
#include "runtime/IOBuf.h"
#include "csm/ContentDbase.h"

#include "csm/ContentCfg.h"
#include "csm/InjectIter.h"
#include "csm/cdbEntries.h"
#include "csm/CdbBodyIter.h"

#include "xstd/gadgets.h"


CdbBodyIter::CdbBodyIter(): theCdb(0), theInjector(0), theEmbedModel(0),
	thePos(-1), theEntryData(-1) {
}

void CdbBodyIter::cdb(ContentDbase *aCdb) {
	Assert(!theCdb && aCdb);
	theCdb = aCdb;
}

void CdbBodyIter::injector(InjectIter *anInjector) {
	Assert(!theInjector && anInjector);
	theInjector = anInjector;
}

void CdbBodyIter::embedContModel(EmbedContMdl *anEmbedModel) {
	Assert(!theEmbedModel && anEmbedModel);
	theEmbedModel = anEmbedModel;
}

void CdbBodyIter::startPos(int aPos) {
	Assert(thePos == -1 && aPos >= 0);
	thePos = aPos;
	theEntryOff = 0;
	theEntryData = 0;
}

void CdbBodyIter::start(WrBuf *aBuf) {
	BodyIter::start(aBuf);
	Assert(theCdb && theCdb->count());
	Assert(thePos >= 0);
	Assert(theEntryOff == 0);
	Should(theEntryData == 0);
	if (theInjector)
		theInjector->start(this);
}

void CdbBodyIter::stop() {
	if (theInjector) {
		theInjector->creator()->putInjector(theInjector);
		theInjector = 0;
	}
}

bool CdbBodyIter::pourMiddle() {
	CdbEntryPrnOpt opt;
	opt.buf = theBuf;
	opt.injector = theInjector;
	opt.embed.model = theEmbedModel;
	opt.embed.rng = &theRng;
	opt.embed.container = theOid;

	while (canPour() && middleSizeLeft()) {
		const CdbEntry *e = theCdb->entry(thePos);

		opt.sizeMax = middleSizeLeft();
		opt.entryOff = theEntryOff;
		opt.entryData = theEntryData;
		bool needMore = false;

		const Size mark = theBuf->contSize();
		const bool poured = e->pour(opt, needMore);
		theBuiltSize += theBuf->contSize() - mark;

		if (poured) {
			if (needMore) {
				theEntryOff = opt.entryOff;
				theEntryData = opt.entryData;
				Should(!theBuf->empty());
				return true;
			} else {
				theEntryOff = 0;
				theEntryData = 0;
				thePos++;
				thePos %= theCdb->count();
			}
		} else {
			Should(theBuf->contSize() == mark); // nothing was poured
			// current entry will never fit, stuff with random data
			theEntryOff = 0;
			theEntryData = 0;
			pourRandom(middleSizeLeft());
		}
	}
	return true;
}

CdbBodyIter *CdbBodyIter::clone() const {
	CdbBodyIter *const i = new CdbBodyIter(*this);
	if (theInjector) {
		i->theInjector = 0;
		i->injector(new InjectIter(*theInjector));
	}
	return i;
}

void CdbBodyIter::calcContentSize() const {
	Should(false);
}
