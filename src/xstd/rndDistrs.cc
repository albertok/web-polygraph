
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iostream.h"

#include "xstd/Assert.h"
#include "xstd/rndDistrs.h"


UnifDistr::UnifDistr(RndGen *aGen, double aLo, double aHi):
	RndDistr(aGen), theLo(aLo), theHi(aHi) {

	Assert(theHi >= theLo);
}

ostream &UnifDistr::print(ostream &os, ArgPrinter p) const {
	os << pdfName() << '(';
	p(os, theLo, 0);
	p(os << ", ", theHi, 1);
	return os << ')';
}

double ExpDistr::trial() {
	return -theMean * log(theGen->trial());
}

double NormDistr::trial() {
	double sum = 0;

	for (int i = 0; i < 12; ++i)
		sum += theGen->trial();

	return theMean + theSDev*(sum - 6.0);
}

ostream &NormDistr::print(ostream &os, ArgPrinter p) const {
	os << pdfName() << '(';
	p(os, theMean, 0);
	p(os << ", ", theSDev, 1);
	return os << ')';
}

// translates mean and std dev into "internal" mu and sigma
// then creates the LognDistr object
LognDistr *LognDistr::ViaMean(RndGen *aGen, double aMean, double aSDev) {

	const double twoLnM = 2 * log(aMean);
	const double lnSum = log(aSDev*aSDev + aMean*aMean);

	const double aMu = twoLnM - lnSum/2;
	const double aSigmaSq = lnSum - twoLnM;

	return new LognDistr(aGen, aMu, aSigmaSq, aMean, aSDev);
}

LognDistr *LognDistr::ViaMu(RndGen *aGen, double aMu, double aSigmaSq) {
	const double realMean = ::exp(aMu + aSigmaSq/2);
	const double w = ::exp(aSigmaSq);
	const double realSDev = sqrt(w * (w - 1) * ::exp(2*aMu)); // ?
	return new LognDistr(aGen, aMu, aSigmaSq, realMean, realSDev);
}

LognDistr::LognDistr(RndGen *generator, double aMu, double aSigmaSq, double aMean, double aSDev):
	NormDistr(generator, aMu, sqrt(aSigmaSq)),
	theRealMean(aMean), theRealSDev(aSDev) {
}

double LognDistr::trial() {
	return ::exp(NormDistr::trial());
}

// translates "internal" mu and sigma into mean
double LognDistr::mean() const {
	return theRealMean;
}

// translates "internal" mu and sigma into std dev
double LognDistr::sdev() const {
	return theRealSDev;
}

ostream &LognDistr::print(ostream &os, ArgPrinter p) const {
	os << pdfName() << '(';
	p(os, theRealMean, 0);
	p(os << ", ", theRealSDev, 1);
	return os << ')';
}


/* Zipf */

// Euler-Mascheroni constant
static const double TheEMConst = 0.57721566490153286060651209;

double ZipfDistr::trial() {
//	return floor(pow(theWorldCap+1, theGen->trial()));
	const double rn = theGen->trial();
	return floor(pow(theWorldCap+1,rn));
}

int ZipfDistr::ltrial(int min, int max) {
	// wrong: (min, max]
	// return min + (int) floor(pow(max-min+1, theGen->trial()));
	// wrong: [min, max)
	// [min, max]
	min--;
	const double rn = theGen->trial();
	return min + (int) floor(pow(max-min+1, rn));
}

double ZipfDistr::omega() const {
	// harmonic sum approximation: log(i) + gamma + 1/(2i)
	return log((double)theWorldCap) + TheEMConst + 1./(2*theWorldCap);
}

ostream &ZipfDistr::print(ostream &os, ArgPrinter p) const {
	os << pdfName() << '(';
	p(os, theWorldCap, 0);
	return os << ')';
}


/* Seq */

SeqDistr::SeqDistr(RndGen *aGen, int aFirstId): RndDistr(aGen), 
	theFirstId(aFirstId), theLastId(aFirstId-1) {
}

double SeqDistr::trial() {
	return ++theLastId;
}

ostream &SeqDistr::print(ostream &os, ArgPrinter p) const {
	os << pdfName() << '(';
	p(os, theFirstId, 0);
	return os << ')';
}
