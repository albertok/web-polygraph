
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_EMBEDCONTMDL_H
#define POLYGRAPH__CSM_EMBEDCONTMDL_H

#include "xstd/Array.h"
#include "base/ObjId.h"
#include "csm/ContentTypeIdx.h"

class WrBuf;
class RndGen;
class RndDistr;
class BodyIter;
class ContentCfg;
class ContentSym;


// simulates embedded content 
// (simulates various *ML tags that allow to "embed" content, to be precise)

class EmbedContMdl {
	public:
		EmbedContMdl();

		void configure(const ContentSym *cfg);

		// called by Server through ContentCfg
		double compContPerCall(const ContentCfg *cc) const;

		// called by ContainerBodyIter
		int embedGoal(RndGen &rng);
		ObjId embedRndOid(const ObjId &oid, int oidCount, RndGen &rng);

		// called by CdbeLink
		ObjId embedCatOid(const ObjId &oid, int oidCount, const String &category);

	protected:
		void configureEmbedCfgs(const ContentSym *cfg);
		bool find(int cfgId, int &idx) const;
		ObjId embedTypedOid(const ObjId &oid, int oidCount, int etype);

	protected:
		Array<ContentCfg*> theCfgs;   // embedded objs configs
		Array<double> theProbs;       // probability of selecting a cfg
		ContentTypeIdx theTypes;      // embedded content type IDs
		RndDistr *theCfgSel;          // selector for cfgs
		RndDistr *theEmbedCount;      // objs per container
		
		double theEmbedCountMean;     // cached mean of theEmbedCount
};

#endif
