
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <limits.h>
#include "xstd/h/iostream.h"

#include "xstd/Assert.h"
#include "xstd/gadgets.h"
#include "xstd/Rnd.h"

// RND_MAX is INT_MAX if sizeof(int) is 4
#define RND_MAX 0x7fffffff

RndGen::State RndGen::TheDefSeed = 1;

// a negative seed will be set (to the default value) before first use
// see trial();
RndGen::RndGen(State aSeed): theSeed(aSeed), theState(aSeed) {
	if (aSeed >= 0)
		seed(aSeed);
	else
		seed(TheDefSeed);
}

void RndGen::seed(State aSeed) {
	Assert(aSeed >= 0);
	theSeed = aSeed;
	theState = initState(aSeed);
}

RndGen::State RndGen::initState(State aSeed) const {
	return aSeed ? aSeed : (RND_MAX/2 | 1);
}

RndGen::State RndGen::state() const {
	return theSeed < 0 ? initState(TheDefSeed) : theState;
}

void RndGen::state(State aState) {
	Assert(aState); // can be negative
	theState = aState;
}

// uniform on [0, 2^31)
RndGen::State RndGen::ltrial() {
	// set initial seed to default if still undefined
	if (theSeed < 0)
		seed(TheDefSeed);

	// one can never get out of zero state
	Assert(theState);

	/*
	 * Compute x[n + 1] = (7^5 * x[n]) mod (2^31 - 1).
	 * From "Random number generators: good ones are hard to find",
	 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
	 * October 1988, p. 1195.
	 *
	 * note: (2^31 - 1) == 127773 * (7^5) + 2836
	 */
	const int h = theState;
	const int hi = h / 127773;
	const int lo = h % 127773;
	const int x = 16807 * lo - 2836 * hi;

	theState = x ? x : RND_MAX;
	return x < 0 ? x + RND_MAX : x;
}


/* RndDistr */

long RndDistr::ltrial() {
	const double t = fabs(trial());
	return (long)MiniMax(0.0, ceil(t), (double)INT_MAX - 1.0);
}

void RndDistr_DefArgPrinter(ostream &os, double arg, int) {
	os << arg;
}

ostream &RndDistr::print(ostream &os, ArgPrinter p) const {
	os << pdfName() << '(';
	p(os, mean(), 0);
	return os << ')';
}
