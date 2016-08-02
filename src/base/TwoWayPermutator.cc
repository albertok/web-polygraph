
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>

#include "xstd/Ring.h"
#include "base/RndPermut.h"
#include "base/TwoWayPermutator.h"


void TwoWayPermutator::configure(int mapSize, RndPermutator &rndPermutator, int seed) {
	Assert(mapSize < INT_MAX/2);

	// direct
	{
		Ring<int> r(mapSize);
		while (r.count() < mapSize)
			r.enqueue(r.count());

		RndGen rng(rndPermutator.permut(seed, rndTwoWayPermutator));
		r.randomize(rng);

		theDirectMap.stretch(mapSize);
		while (theDirectMap.count() < mapSize)
			theDirectMap.append(r.dequeue());
	}

	// reverse
	theReverseMap.resize(mapSize);
	for (int i = 0; i < theDirectMap.count(); ++i)
		theReverseMap[theDirectMap[i]] = i;
}

// returns 0 <= m < mapSize
int TwoWayPermutator::directLookup(int n) const {
	if (Should(0 <= n && n < theDirectMap.count()))
		return theDirectMap[n];
	else
		return 0;
}

// returns 0 <= n < mapSize
int TwoWayPermutator::reverseLookup(int m) const {
	if (Should(0 <= m && m < theReverseMap.count()))
		return theReverseMap[m];
	else
		return 0;
}

