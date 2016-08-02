
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CDBBODYITER_H
#define POLYGRAPH__CSM_CDBBODYITER_H

#include "csm/BodyIter.h"

class ContentDbase;
class InjectIter;
class EmbedContMdl;

class CdbBodyIter: public BodyIter {
	public:
		CdbBodyIter();
		virtual CdbBodyIter *clone() const;

		virtual void start(WrBuf *aBuf);
		virtual void stop();

		void cdb(ContentDbase *aCdb);
		void injector(InjectIter *anInjector);
		void embedContModel(EmbedContMdl *anEmbedModel);
		void startPos(int aPos);

	protected:
		virtual bool pourMiddle();
		virtual void calcContentSize() const;

	protected:
		ContentDbase *theCdb;    // the source of content body
		InjectIter *theInjector; // inserts foreign content if configured
		EmbedContMdl *theEmbedModel; // CdbeLink uses this to create links
		int thePos;              // current position in the content database
		Size theEntryOff;        // poured from the current cdb entry
		int theEntryData;        // saved from the current cdb entry
};

#endif
