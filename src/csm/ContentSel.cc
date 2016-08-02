
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Rnd.h"
#include "base/ObjId.h"
#include "xstd/TblDistr.h"
#include "base/RndPermut.h"
#include "csm/ContentCfg.h"
#include "csm/ContentMgr.h"
#include "runtime/LogComment.h"

#include "pgl/ContentSym.h"
#include "pgl/ServerSym.h"

#include "csm/ContentSel.h"

#include "xstd/gadgets.h"


ContentSel::ContentSel(): theDirAccSel(0) {
}

ContentSel::~ContentSel() {
	delete theDirAccSel;
}

void ContentSel::configure(const ServerSym *cfg) {
	Array<ContentCfg*> ccfgs;

	theSrvKind = cfg->kind();

	// get "global" content information
	Array<ContentSym*> csyms;
	if (!cfg->contents(csyms, theContProbs) || !csyms.count()) {
		cerr << cfg->loc() << "no content specs for server " << cfg->kind() << endl;
		exit(-2);
	}
	Assert(csyms.count() == theContProbs.count());
	TheContentMgr.get(csyms, theContents);

	// get direct access info
	Array<ContentSym*> dasyms;
	if (!cfg->directAccess(dasyms, theDaProbs) || !dasyms.count()) {
		cerr << cfg->loc() << "no direct_access specs for server " << cfg->kind() << endl;
		exit(-2);
	}
	TheContentMgr.get(dasyms, theDirectAccess);

	// check that all direct access cfgs are unique
	// and build cprobsViaDa map
	if (!theDaProbs.count())
		computeDaProbs(dasyms);

	// make selector for direct access content
	theDirAccSel = TblDistr::FromDistrTable("da-sel", theDaProbs);
	Assert(theDirAccSel);
	theDirAccSel->rndGen()->seed(LclPermut(rndContentSel));

	ContentTypeIdx::configure(theContents);
}

void ContentSel::computeDaProbs(const Array<ContentSym*> &dasyms) {
	Assert(theDirectAccess.count());

	Array<double> cprobsViaDa;
	cprobsViaDa.stretch(theDirectAccess.count());
	{for (int d = 0; d < theDirectAccess.count(); ++d) {
		ContentCfg *da = theDirectAccess[d];
		int count = 0;
		for (int c = 0; c < theContents.count(); ++c) {
			if (theContents[c]->id() == da->id()) {
				cprobsViaDa.append(theContProbs[c]);
				count++;
			}
		}
		if (count != 1) {
			cerr << dasyms[d]->loc() 
				<< "direct_access spec below counted " << count
				<< " times in the global contents specs (must be 1):"
				<< endl << *dasyms[d] << endl;
			exit(-2);
		}
	}}

	// compute direct access probabilities using known content distribution
	double daProb = 0;
	{for (int d = 0; d < theDirectAccess.count(); ++d)
		daProb += cprobsViaDa[d];
	}

	theDaProbs.stretch(theDirectAccess.count());
	{for (int d = 0; d < theDirectAccess.count(); ++d)
		theDaProbs.append(cprobsViaDa[d]/daProb); // contribution of d
	}
}

void ContentSel::reportCfg(ostream &os) const {
	// calculate expected global content distribution
	// and average cachability of pages
	Array<double> expCProbs(theContents.count());
	double expCProbsSum = 0;
	for (int c = 0; c < theContents.count(); ++c) {
		const ContentCfg *cc = theContents[c];

		// compute average number of oids of type c generated per da
		double contPerDa = 0;
		for (int d = 0; d < theDirectAccess.count(); ++d) {
			ContentCfg *da = theDirectAccess[d];
			//cerr << here << "da[" << d << "]*" << theDaProbs[d] << '/' << daProb << ": " << da->compContPerCall(cc) << endl;
			contPerDa += da->compContPerCall(cc) * theDaProbs[d];
		}
		
		// check that content type is accessible
		if (contPerDa <= 0) {
			cerr << ": warning: content `" << cc->kind() << "' is not accessible "
				<< "(i.e., is not in direct_access closure)." << endl;
		}

		expCProbs.put(contPerDa, c);
		expCProbsSum += contPerDa;
	}

	// finilize calculation of expCProbs
	// and compute average cachability of pages
	double chbSum = 0;
	double sizeSum = 0;

	// report planned and expected global content distributions
	os << "\tServer " << theSrvKind << ':' << endl;
	os << '\t' << setw(15) << "content"
		<< ' ' << setw(10) << "planned%" 
		<< ' ' << setw(10) << "likely%" 
		<< ' ' << setw(10) << "error%" 
		<< ' ' << setw(10) << "mean_sz_bytes" 
		<< endl;
	{for (int c = 0; c < theContents.count(); ++c) {
		const ContentCfg *cc = theContents[c];
		expCProbs[c] /= expCProbsSum;
		chbSum += expCProbs[c] * cc->theChbRatio;
		const double size = cc->repSizeMean();
		sizeSum += expCProbs[c] * size;
		os << '\t' << setw(15) << cc->kind()
			<< ' ' << setw(10) << 100*theContProbs[c]
			<< ' ' << setw(10) << 100*expCProbs[c]
			<< ' ' << setw(10) << Percent(expCProbs[c]-theContProbs[c], theContProbs[c])
			<< ' ' << setw(10) << size
			<< endl;
	}}
	os << "\texpected average server-side cachability: " << 100*chbSum << '%' << endl;
	os << "\texpected average server-side object size: " << sizeSum << "Bytes" << endl;
}

const ContentCfg &ContentSel::getDir() {
	const int tid = (int)theDirAccSel->trial();
	Assert(tid >= 0 && tid < theDirectAccess.count());
	return *theDirectAccess[tid];
}
