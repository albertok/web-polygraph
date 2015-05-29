
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_RND_H
#define POLYGRAPH__XSTD_RND_H

#include "xstd/h/iosfwd.h"
#include "xstd/h/math.h"

// simple random generator
// every generator gets its own seed and state
// no virtual stuff as we do not need more than one type of generators
class RndGen {
	public:
		typedef long State;
		static State TheDefSeed;   // default seed

	public:
		RndGen(State aSeed = -1);

		// uniform[0,1)
		double trial() { return ltrial()/2147483648.0; }
		double operator ()() { return trial(); }

		// uniform[0,2147483648)
		long ltrial();

		// equiprob[lo,hi); "hi" is excluded inless hi==lo
		double operator ()(double lo, double hi) { return lo + (hi-lo)*trial(); }
		int operator ()(int lo, int hi) { return hi != lo ? lo + ltrial()%(hi-lo) : lo; }

		// Bernoulli event with a given probability
		bool event(double prob) { return trial() < prob; }

		// get/change seed/state
		State state() const;
		void state(State state);
		void seed(State seed);
		void reset() { seed(theSeed); }

	protected:
		State initState(State aSeed) const;

	protected:
		State theSeed;  // initial seed
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
