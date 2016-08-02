
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CONTAINERBODYITER_H
#define POLYGRAPH__CSM_CONTAINERBODYITER_H

#include "csm/BodyIter.h"

class EmbedContMdl;

class ContainerBodyIter: public BodyIter {
	public:
		ContainerBodyIter();
		virtual ContainerBodyIter *clone() const;

		virtual void start(WrBuf *aBuf);

		void embedContModel(EmbedContMdl *aModel);

	protected:
		virtual bool pourMiddle();
		virtual void calcContentSize() const;

		// writes embedded oid tag and returns true if something was written
		bool embed();

	protected:
		EmbedContMdl *theModel;
		int theEmbedGoal;     // how many objects to embed
		int theEmbedCount;    // number of objects embedded so far
		Size theEmbedDist;    // ~ distance between embed tags
};

#endif
