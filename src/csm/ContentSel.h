
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CONTENTSEL_H
#define POLYGRAPH__CSM_CONTENTSEL_H

#include "xstd/Array.h"
#include "csm/ContentTypeIdx.h"

class RndDistr;
class ContentCfg;
class ContentSym;
class ServerSym;

// selects content configuration based on object identifier
class ContentSel: public ContentTypeIdx {
	public:
		ContentSel();
		~ContentSel();

		void configure(const ServerSym *cfg);

		const Array<ContentCfg*> &contents() { return theContents; }
		const ContentCfg &getDir();

		void reportCfg(ostream &os) const;
		
	protected:
		void computeDaProbs(const Array<ContentSym*> &dasyms);

	protected:
		String theSrvKind;
		Array<ContentCfg*> theContents;      // all content
		Array<double> theContProbs;
		Array<double> theDaProbs;

		Array<ContentCfg*> theDirectAccess;  // directly accessible content
		RndDistr *theDirAccSel;              // selector for the above
};

#endif
