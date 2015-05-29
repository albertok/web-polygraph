
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_RNDDISTRS_H
#define POLYGRAPH__XSTD_RNDDISTRS_H

#include "xstd/Rnd.h"

// constant(mean) distribution
class ConstDistr: public RndDistr
{
	public:
		ConstDistr(RndGen *aGen, double aMean): RndDistr(aGen), theMean(aMean) {}

		virtual const char *pdfName() const { return "const"; }

		virtual double mean() const { return theMean; }
		virtual double sdev() const { return 0; }

		virtual double trial() { return theMean; }

	protected:
		double theMean;
};


// uniform[lo,hi) distribution
class UnifDistr: public RndDistr
{
	public:
		UnifDistr(RndGen *aGen, double aLo, double aHi);

		virtual const char *pdfName() const { return "unif"; }

		virtual double mean() const { return (theLo+theHi)/2; }
		virtual double sdev() const { return range()/sqrt(12.); }
		double range() const { return theHi-theLo; }

		virtual double trial() { return theLo + range()*theGen->trial(); }

		virtual ostream &print(ostream &os, ArgPrinter p = &RndDistr_DefArgPrinter) const;

	protected:
		double theLo;
		double theHi;
};

// exponential(mean) distribution
class ExpDistr: public RndDistr
{
	public:
		ExpDistr(RndGen *aGen, double aMean): RndDistr(aGen), theMean(aMean) {}

		virtual const char *pdfName() const { return "exp"; }

		virtual double mean() const { return theMean; }
		virtual double sdev() const { return theMean; }

		virtual double trial();

	protected:
		double theMean;
};

// normal(mean, sdev) distribution
class NormDistr: public RndDistr
{
	public:
		NormDistr(RndGen *aGen, double aMean, double aSDev): RndDistr(aGen), theMean(aMean), theSDev(aSDev) {}

		virtual const char *pdfName() const { return "norm"; }

		virtual double mean() const { return theMean; }
		virtual double sdev() const { return theSDev; }

		virtual double trial();

		virtual ostream &print(ostream &os, ArgPrinter p = &RndDistr_DefArgPrinter) const;

	protected:
		double theMean;
		double theSDev;
};

// lognormal(mu, sigmaSq) distribution
class LognDistr: public NormDistr
{
	public:
		// creates using mean/sdev
		static LognDistr *ViaMean(RndGen *aGen, double aMean, double aSDev);
		// creates using mu/sigma
		static LognDistr *ViaMu(RndGen *aGen, double aMu, double aSigmaSq);

	public:
		virtual const char *pdfName() const { return "logn"; }

		virtual double mean() const;
		virtual double sdev() const;

		virtual double trial();

		virtual ostream &print(ostream &os, ArgPrinter p = &RndDistr_DefArgPrinter) const;

	protected:
		LognDistr(RndGen *generator, double aMu, double aSigmaSq, double aMean, double aSDev);

	protected:
		double theRealMean;
		double theRealSDev;
};

// classic Zipf(#objects) distribution (exponent == 1)
class ZipfDistr: public RndDistr
{
	public:
		ZipfDistr(RndGen *aGen, int aWorldCap): RndDistr(aGen), theWorldCap(aWorldCap) {}

		virtual const char *pdfName() const { return "zipf"; }

		virtual double mean() const { return -1; } // infinite mean
		virtual double sdev() const { return 0; }  // infinite dev?
		double omega() const;

		virtual double trial();
		int ltrial(int min, int max);

		virtual ostream &print(ostream &os, ArgPrinter p = &RndDistr_DefArgPrinter) const;

	protected:
		int theWorldCap;
};

// generates sequential ids: 1, 2, 3, ...
class SeqDistr: public RndDistr
{
	public:
		SeqDistr(RndGen *aGen, int aFirstId = 1);

		virtual const char *pdfName() const { return "seq"; }

		virtual double mean() const { return -1; } // infinite mean
		virtual double sdev() const { return 0; }  // infinite dev?

		virtual double trial();

		virtual ostream &print(ostream &os, ArgPrinter p = &RndDistr_DefArgPrinter) const;

	protected:
		int theFirstId;
		int theLastId;
};

#endif
