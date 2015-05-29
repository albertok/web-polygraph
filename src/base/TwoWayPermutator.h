
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_TWOWAYPERMUTATOR_H
#define POLYGRAPH__BASE_TWOWAYPERMUTATOR_H

#include "xstd/Array.h"

class RndPermutator;

// a 1:1 map of random numbers in a given range
// with direct and reverse lookup ability
class TwoWayPermutator {
	public:
		void configure(int mapSize, RndPermutator &rndPermutator, int seed);

		int directLookup(int n) const;  // returns 0 <= m < theMapSize
		int reverseLookup(int m) const; // returns 0 <= n < theMapSize

	protected:
		Array<int> theDirectMap;
		Array<int> theReverseMap;
};

#endif
