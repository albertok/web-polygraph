
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CONTENTTYPEIDX_H
#define POLYGRAPH__CSM_CONTENTTYPEIDX_H

#include "xstd/Array.h"

class ContentCfg;
class ObjId;
class Area;

// stores content type Ids for SrvCfg and ContentSel to use
class ContentTypeIdx: public Array<int> {
	public:
		void configure(const Array<ContentCfg*> &ccfgs);

		bool hasContType(int tid) const;
		bool findEmbedContType(const Area &category, int &etype) const;
};

#endif
