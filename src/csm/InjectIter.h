
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_INJECTITER_H
#define POLYGRAPH__CSM_INJECTITER_H

#include "xstd/Rnd.h"

class RndDistr;
class ContentCfg;
class CdbBodyIter;
class TextDbase;
class WrBuf;


class InjectIter {
	public:
		InjectIter() { reset(); }

		void reset();

		void creator(ContentCfg *aCreator) { theCreator = aCreator; }
		ContentCfg *creator() { return theCreator; }

		void textDbase(TextDbase *aTdb) { theTdb = aTdb; }
		void gap(RndDistr *aGap) { theInjGap = aGap; }

		void start(CdbBodyIter *aBodyIter);
		void inject(WrBuf &buf, Size areaOff, Size areaLen);

	protected:
		ContentCfg *theCreator;
		TextDbase *theTdb;
		CdbBodyIter *theBodyIter;

		RndDistr *theInjGap;
		Size theInjOff;
		
		RndGen theTdbPosGen;
		int theTdbPos;      // pos in text database

		bool incInjOff;     // generate new injection pos
};

#endif
