
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>

#include "xstd/Map.h"
#include "xstd/Rnd.h"
#include "xstd/String.h"
#include "base/RndPermut.h"

static RndPermutator *TheLclPermut = 0;
static RndPermutator *TheGlbPermut = 0;


RndPermutator::RndPermutator(int setSize): theTable(0), theTableCap(0) {
	if (setSize)
		configure(setSize);
}

RndPermutator::~RndPermutator() {
	configure(0);
}

void RndPermutator::configure(int setSize, int seed) {
	delete[] theTable;

	if (setSize > 0) {
		theTable = new int[theTableCap = setSize];
		reseed(seed);
	} else {
		theTable = 0;
		theTableCap = 0;
	}
}

// fill set with "good" values
void RndPermutator::reseed(int seed) {
	RndGen rng(seed);
	for (int j = 0; j < theTableCap; ++j)
		theTable[j] = rng.trial32();
	for (int i = 0; i < theTableCap; ++i)
		swap(i, rng(0, theTableCap));
}

int RndPermutator::permut(const int64_t n, const int64_t m) const {
	const int64_t a = n / theTableCap;
	const int64_t b = n % theTableCap;
	const int offset = item(a) - item(m);
	return item(offset + b);
}


static
RndGen *GetRndGen(const String &group, int seed) {
	static PtrMap<RndGen*> gens;
	RndGen *gen = 0;
	if (!gens.find(group, gen)) {
		gen = new RndGen(seed);
		gens.add(group, gen);
	}
	return gen;
}

RndGen *GlbRndGen(const String &group) {
	const String g = "glb-" + group;
	return GetRndGen(g, GlbPermut(g.hash()));
}

RndGen *LclRndGen(const String &group) {
	const String g = "lcl-" + group;
	return GetRndGen(g, LclPermut(g.hash()));
}

RndPermutator &LclPermut() {
	Assert(TheLclPermut);
	return *TheLclPermut;
}

RndPermutator &GlbPermut() {
	Assert(TheGlbPermut);
	return *TheGlbPermut;
}


/* initialization */

int RndPermutfInit::TheUseCount = 0;

void RndPermutfInit::init() {
	const int defSize = 1048573; // 82,025-th prime number! (1M=1048576)
	TheLclPermut = new RndPermutator(defSize);
	TheGlbPermut = new RndPermutator(defSize);
}

void RndPermutfInit::clean() {
	delete TheLclPermut; TheLclPermut = 0;
	delete TheGlbPermut; TheGlbPermut = 0;
}
