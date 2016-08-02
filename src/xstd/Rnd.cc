
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <limits.h>
#include "xstd/h/iostream.h"

#include "xstd/Assert.h"
#include "xstd/gadgets.h"
#include "xstd/Rnd.h"

RndGen::Seed RndGen::TheDefSeed = 0;

void RndGen::DefSeed(const Seed aSeed) {
	Must(aSeed >= 0);
	TheDefSeed = aSeed;
}

RndGen::RndGen(const Seed aSeed) {
	if (aSeed >= 0)
		seed(aSeed);
	else
		seed(TheDefSeed);
}

void RndGen::seed(const Seed aSeed) {
	Assert(aSeed >= 0);
	// Zero seed is valid, but the first trial would return zero.
	// Avoid this correlation.  Using int_max/2 is slightly better
	// than 1: If you are testing rng and trying each seed one by
	// one, seeds 0 and 1 will not produce the same results.
	theSeed = aSeed > 0 ? aSeed : (numeric_limits<uint32_t>::max() / 2 | 1);

	// Seed is 64bit, but they usually come from permutators that
	// use 32bit seeds, so initialize 16-32 bits in the state with
	// 8-24 bits from the seed.
	// TODO: use 64bit integers in permutators?
	theState.s0 = theSeed & 0xFFFF;
	theState.s1 = (theSeed >> 8) & 0xFFFF;
	theState.s2 = (theSeed >> 16) & 0xFFFF;
}

RndGen::Seed RndGen::state() const {
	return (static_cast<Seed>(theState.s2) << 32) |
		(static_cast<Seed>(theState.s1) << 16) |
		theState.s0;
}

void RndGen::state(const Seed aSeed) {
	theState.s0 = aSeed & 0xFFFF;
	theState.s1 = (aSeed >> 16) & 0xFFFF;
	theState.s2 = (aSeed >> 32) & 0xFFFF;
}

uint32_t RndGen::trial32u() {
	/*
	 * Linear congruential generator:
	 *
	 *   x[n + 1] = (a * x[n] + c) mod m
	 *
	 * Where:
	 *
	 *   m = 2^48 = 281474976710656
	 *   a = 25214903917 = 0x5DEECE66D
	 *   c = 11 = 0xB
	 *
	 * Aka the Unix rand48() generator.
	 */
	const uint16_t a0 = 0xE66D;
	const uint16_t a1 = 0xDEEC;
	const uint16_t a2 = 0x5;
	const uint16_t c = 0xB;

	State newState;
	uint32_t x = theState.s0 * a0 + c;
	newState.s0 = x & 0xFFFF;
	x >>= 16;

	// may overflow, but we do not care about bits above 32
	x += theState.s0 * a1 + theState.s1 * a0;
	newState.s1 = x & 0xFFFF;
	x >>= 16;

	x += theState.s0 * a2 + theState.s1 * a1 + theState.s2 * a0;
	newState.s2 = x & 0xFFFF;

	theState = newState;

	// take 32 higher bits because they have better randomness
	return (theState.s2 << 16) | theState.s1;
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
