
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_RNDPERMUT_H
#define POLYGRAPH__BASE_RNDPERMUT_H

#include <limits>

#include "xstd/LibInit.h"


// We often need to convert a number to some uncorrellated uniformly
// distributed random number. Using seed-and-trial approach with a r.n.g. 
// does not work well if input stream of numbers is sequential and long.
// The permutator is also useful for seeding a r.n.g.

// Current permutation allows for a given [small] number of high quality 
// random values. The size of the output set is a parameter.

class RndPermutator {
	public:
		RndPermutator(int setSize = 0);
		~RndPermutator();

		// changes the permutation set; expensive;
		// can be called multiple times, any time
		// using prime numbers for set size is recommended
		void configure(int setSize, int seed = 1);

		// set size remains unchanged, but contents changes
		void reseed(int seed);
		
		// use 2nd param if you need to map two numbers into one uniform var
		int permut(const int64_t n, const int64_t m = 0) const;

	protected:
		inline void swap(int x, int y);
		inline int item(const int64_t idx) const;

	protected:
		int *theTable;   // good random numbers
		int theTableCap;
};

// constant "offsets" to use as a second parameter to RndPermut
// when the first parameter is the same for a set of calls
enum { rndNone = 0, rndContentSel, rndUnused1, 
	rndContentPfx, rndContentExt,
	rndBodyIter, rndInjTbdPos, rndInjOff, rndInjProb,
	rndHotSetPos,
	rndRepOlc, rndRepSize, rndRepCach, rndRepCheckNeed, rndCdbStart,
	rndSharedContent, rndUniqueContent,
	rndEmbedContType,
	rndTwoWayPermutator,
	rndMembershipRangeBeg,
	rndArraySymSelector, rndPglSemxAssignment, rndPglSemxSelectItems,
	rndPglSemxIsDistr, rndRobotSymReqInterArrival,
	rndSslSeed, rndSslSessionCache,
	rndCookieSend, rndCookieCount, rndCookieSize,
	rndReqBody, rndWorldSel, rndProtocolSel, rndHttpHeaders,
	rndRamFilesStart,
	rndEnd
};

// generate one seeded r.n.g. per group
class RndGen;
class String;
extern RndGen *GlbRndGen(const String &group);
extern RndGen *LclRndGen(const String &group);

extern RndPermutator &LclPermut(); // each process gets its own rnd numbers
extern RndPermutator &GlbPermut(); // all processes share this set of numbers

inline
int LclPermut(const int64_t n, const int64_t m = 0) {
	return LclPermut().permut(n, m);
}

inline
int GlbPermut(const int64_t n, const int64_t m = 0) {
	return GlbPermut().permut(n, m);
}



/* inlined methods */

inline 
void RndPermutator::swap(int x, int y) {
	const int h = theTable[x];
	theTable[x] = theTable[y];
	theTable[y] = h;
}

inline 
int RndPermutator::item(const int64_t idx) const {
	return idx >= 0 ?
		theTable[idx % theTableCap] :
		theTable[(idx + numeric_limits<int64_t>::max()) % theTableCap];
}


LIB_INITIALIZER(RndPermutfInit)

#endif
