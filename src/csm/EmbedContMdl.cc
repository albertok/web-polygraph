
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "runtime/Farm.h"
#include "csm/BodyIter.h"
#include "csm/ContentCfg.h"
#include "csm/ContentMgr.h"

#include "pgl/ContentSym.h"

#include "csm/EmbedContMdl.h"
#include "base/rndDistrStat.h"


EmbedContMdl::EmbedContMdl(): theCfgSel(0), theEmbedCount(0), theEmbedCountMean(-1) {
}

void EmbedContMdl::configure(const ContentSym *cfg) {
	Assert(!theEmbedCount);

	if ((theEmbedCount = cfg->embedCount()))
		theEmbedCountMean = RndDistrStat(theEmbedCount).mean();

	configureEmbedCfgs(cfg);

	if (theEmbedCount && !theCfgs.count()) {
		cerr << cfg->loc() 
			<< "the number of embedded objects per container is specified, "
			<< "but object content types are undefined"
			<< endl;
		exit(-2);
	}

	if (!theEmbedCount && theCfgs.count()) {
		cerr << cfg->loc() 
			<< "object content types are specified, "
			<< "but the number of embedded objects per container is not"
			<< endl;
		exit(-2);
	}
}

double EmbedContMdl::compContPerCall(const ContentCfg *cc) const {
	int idx = 0;
	if (find(cc->id(), idx))
		return theEmbedCountMean * theProbs[idx];
	return 0.0;
}

int EmbedContMdl::embedGoal(RndGen &rng) {
	if (!theEmbedCount)
		return 0;
	theEmbedCount->rndGen(&rng); // note: we do not reset rng back
	return (int)theEmbedCount->trial();
}

ObjId EmbedContMdl::embedRndOid(const ObjId &oid, int oidCount, RndGen &rng) {
	// select the embeded content type
	// XXX: consistency: pick name first; type should be deterimed by name
	theCfgSel->rndGen(&rng);   // note: we do not reset rng back
	const int idx = (int)theCfgSel->trial();
	Assert(0 <= idx && idx < theCfgs.count());
	const ContentCfg *cc = theCfgs[idx];

	return embedTypedOid(oid, oidCount, cc->id());
}

ObjId EmbedContMdl::embedCatOid(const ObjId &oid, int oidCount, const String &category) {
	int etype = -1;
	if (!Should(theTypes.findEmbedContType(category.area(0), etype)))
		clog << here << "invalid embedded type category: " << category << endl;
	return embedTypedOid(oid, oidCount, etype);
}

ObjId EmbedContMdl::embedTypedOid(const ObjId &oid, int oidCount, int etype) {
	ObjId eoid = oid;
	eoid.world(oid.world().genMutant());
	Should(etype >= 0); // how to check upper boundary?
	eoid.type(etype);

	// with large numbers, embedded objects will start sharing containers
	static int maxOidCountSeen = 500;
	if (oidCount > maxOidCountSeen) {
		if (oidCount - maxOidCountSeen > maxOidCountSeen/10) {
			cerr << here << "warning: saw " << oidCount <<
				" embedded objects in a single container, possibly causing " <<
				"container sharing among embedded objects" << endl;
		}
		maxOidCountSeen = oidCount;
	}

	const int off = oidCount + 1;
	eoid.name(Abs((oid.name() << 8) | off));

	return eoid;
}

void EmbedContMdl::configureEmbedCfgs(const ContentSym *cfg) {
	Array<ContentSym*> syms;
	cfg->contains(syms, theCfgSel, theProbs);

	theCfgs.stretch(syms.count());
	for (int i = 0; i < syms.count(); ++i) {
		ContentCfg *cfg = TheContentMgr.get(syms[i]);
		theCfgs.append(cfg);
	}

	theTypes.configure(theCfgs);
}

bool EmbedContMdl::find(int cfgId, int &idx) const {
	for (idx = 0; idx < theCfgs.count(); ++idx) {
		if (theCfgs[idx]->id() == cfgId)
			return true;
	}
	return false;
}
