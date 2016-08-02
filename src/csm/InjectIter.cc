
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "runtime/IOBuf.h"
#include "csm/CdbBodyIter.h"
#include "csm/TextDbase.h"
#include "csm/InjectIter.h"

#include "base/RndPermut.h"
#include "xstd/gadgets.h"


void InjectIter::reset() {
	theCreator = 0;

	theAlgorithm = ialgNone;
	theTdb = 0;
	theBodyIter = 0;

	theInjGap = 0;
	theInjOff = Size();

	theTdbPos = -1;
	incInjOff = true;
}

void InjectIter::start(CdbBodyIter *aBodyIter) {
	Assert(theAlgorithm);
	Assert(theTdb && theTdb->count());
	Assert(!theBodyIter && aBodyIter);
	theBodyIter = aBodyIter;
	theTdbPosGen.seed(GlbPermut(theBodyIter->contentHash(), rndInjTbdPos));
	theInjGap->rndGen()->seed(GlbPermut(theBodyIter->contentHash(), rndInjOff));
	theInjOff = 0;
}

void InjectIter::inject(WrBuf &buf, Size areaOff, Size areaLen) {
	do {
		// get next injection position if needed
		if (incInjOff) {
			// XXX: this may overflow, borrow from SrvXact
			theInjOff += (Size)(int)rint(theInjGap->trial());
			incInjOff = false;
		}

		// get next injection text if needed
		if (theTdbPos < 0)
			theTdbPos = theTdbPosGen(0, theTdb->count());

		const Size injLen = theTdb->len(theTdbPos);

		// will it fit?
		if (injLen <= areaLen) {

			const Size bufEnd = theBodyIter->builtSize() + areaOff + areaLen;
			const Size offMax = bufEnd - injLen;

			// is it time to inject?
			if (theInjOff <= offMax) {
				const Size bufOff = Min(areaLen, bufEnd - theInjOff);
				Assert(buf.contSize() >= bufOff);
				buf.overwrite(buf.contSize() - bufOff, 
					theTdb->str(theTdbPos), injLen);

				// reset cache after we used up the values
				incInjOff = true;
				theTdbPos = -1;

				// prevent injections from overlapping
				theInjOff = bufEnd - bufOff + injLen;
			}
		}
	} while (incInjOff);
}
