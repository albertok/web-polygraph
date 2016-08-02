
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_RNDBODYITER_H
#define POLYGRAPH__CSM_RNDBODYITER_H

#include "csm/BodyIter.h"

class RndBodyIter: public BodyIter {
	public:
		virtual RndBodyIter *clone() const;

	protected:
		virtual bool pourMiddle();
		virtual void calcContentSize() const;
};

#endif
