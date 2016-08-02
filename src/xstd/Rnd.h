
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_RND_H
#define POLYGRAPH__XSTD_RND_H

#include <limits>

#include "xstd/h/iosfwd.h"
#include "xstd/h/math.h"
#include "xstd/h/stdint.h"

// simple random generator
// every generator gets its own seed and state
// no virtual stuff as we do not need more than one type of generators
class RndGen {
	public:
		typedef int64_t Seed;
		static void DefSeed(const Seed aSeed);

	public:
		// a negative seed will be set (to the default value) before first use
		RndGen(const Seed aSeed = -1);

		// uniform[0,1)
		double trial() { return trial32()/2147483648.0; }
		double operator ()() { return trial(); }

		// uniform[0,2^32)
		uint32_t trial32u();
		// uniform[0,2^31)
		int32_t trial32() { return trial32u() & numeric_limits<int32_t>::max(); }

		// uniform[0,2^64)
		uint64_t trial64u() { return (static_cast<uint64_t>(trial32u()) << 32) | trial32u(); }
		// uniform[0,2^63)
		int64_t trial64() { return trial64u() & numeric_limits<int64_t>::max(); }

		// equiprob[lo,hi); "hi" is excluded inless hi==lo
		double operator ()(double lo, double hi) { return lo + (hi-lo)*trial(); }
		int32_t operator ()(const int32_t lo, const int32_t hi) { return hi != lo ? lo + trial32u()%(hi-lo) : lo; }
		int64_t operator ()(const int64_t lo, const int64_t hi) { return hi != lo ? lo + trial64u()%(hi-lo) : lo; }

		// Bernoulli event with a given probability
		bool event(double prob) { return trial() < prob; }

		// get/change seed/state
		void seed(const Seed aSeed);
		void reset() { seed(theSeed); }
		// We should return and accept State instead of Seed here, but
		// current parsing/printing code cannot handle State type and
		// we do not really lose any information during this conversion.
		Seed state() const;
		void state(const Seed aSeed); // use only with values returned by state()

	protected:
		struct State {
			uint16_t s0; // lower bits
			uint16_t s1;
			uint16_t s2; // higher bits
		};

		static Seed TheDefSeed; // default seed
		Seed theSeed; // initial seed
		State theState; // current state
};

// old compilers prevent from making this a static method (XXX: redesign)
extern void RndDistr_DefArgPrinter(ostream &os, double arg, int idx);

// abstract distribution
// allows for sharing of one random generator among several distributions
class RndDistr {
	public:
		typedef void (*ArgPrinter)(ostream &os, double arg, int idx);

	public:
		RndDistr(RndGen *aGen): theGen(aGen) {}
		virtual ~RndDistr() {} // may be leaking; we do not delete theGen

		virtual const char *pdfName() const = 0; // type of pdf (e.g., exp)
		RndGen *rndGen() { return theGen; }
		virtual void rndGen(RndGen *aGen) { theGen = aGen; }

		// returns next pseudo random number
		double operator ()() { return trial(); }
		virtual double trial() = 0;
		long ltrial();

		virtual double mean() const = 0; // mean
		virtual double sdev() const = 0;  // standard deviation
		double var() const { return sdev()*sdev(); }  // variance

		virtual ostream &print(ostream &os, ArgPrinter p = &RndDistr_DefArgPrinter) const;

	protected:
		RndGen *theGen;
};

inline
ostream &operator <<(ostream &os, const RndDistr &d) { return d.print(os); }

#endif
