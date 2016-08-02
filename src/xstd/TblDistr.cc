
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iostream.h"

#include "xstd/Assert.h"
#include "xstd/TblDistr.h"
#include "xstd/gadgets.h"

// used internally by TblDistr to handle extras
// a single-point bin
class TblDistrExtras: public TblDistrBin {
	public:
		TblDistrExtras(TblDistr *const anOwner, TblDistr *const aDist);
		virtual ~TblDistrExtras();

		virtual void rndGen(RndGen *aGen);

		virtual IntervalSide left() const;
		virtual IntervalSide right() const;

		virtual double trial();
		virtual ostream &print(ostream &os) const;

	protected:
		TblDistr *theDist;
};


/* TblDistr */

int TblDistr::TheIdxLmt = 10*1024;

TblDistr::TblDistr(const String &aName):
	RndDistr(new RndGen), theExtras(0), theName(aName) {
}

TblDistr::~TblDistr() {
	while (theBins.count()) delete theBins.pop();
}

void TblDistr::rndGen(RndGen *aGen) {
	RndDistr::rndGen(aGen);
	if (theExtras)
		theExtras->rndGen(aGen);
}

double TblDistr::trial() {
	Assert(theBinSelector.count()); // this must be compiled

	const int selIdx = theGen->trial32u() % theBinSelector.count();
	const int binIdx = theBinSelector[selIdx];
	Assert(binIdx < theBins.count());
	TblDistrBin *bin = theBins[binIdx];
	Assert(bin);
	return bin->trial();
}

void TblDistr::addBin(TblDistrBin *bin) {
	Assert(bin);
	Assert(theBins.count() < (1 << 15)); // selector uses short
	Assert(theBins.count() < TheIdxLmt); // see adj_lmt
	Assert(!theBinSelector.count()); // re-compiling is not recommended

	bin->owner(this);
	theBins.append(bin);
}

bool TblDistr::overlapping(const TblDistrBin *bin) const {
	
	// check if the given bin sides belong to any of our bins
	if (covered(bin->left()) || covered(bin->right()))
		return true;

	// check if any of our sides belong to the given bin
	for (int i = 0; i < theBins.count(); ++i) {
		if (bin->covered(theBins[i]->left()))
			return true;
		if (bin->covered(theBins[i]->right()))
			return true;
	}

	return theExtras && theExtras->overlapping(bin);
}

// internal and not recursive
bool TblDistr::covered(const IntervalSide &val) const {
	for (int i = 0; i < theBins.count(); ++i) {
		if (theBins[i]->covered(val))
			return true;
	}
	return false;
}

int TblDistr::binCount() const {
	return theBins.count() +
		(theExtras ? theExtras->binCount() : 0);
}

int TblDistr::idxCount() const {
	return theBinSelector.count() +
		(theExtras ? theExtras->idxCount() : 0);
}

int TblDistr::idxDepth() const {
	return 1 + (theExtras ? theExtras->idxDepth() : 0);
}

// internal: sets relative contributions and manages Extras
void TblDistr::preprocess() {
	Assert(!theExtras);
	const double sum = totAbsContrib();
	Array<TblDistrBin*> core, extras;

	// set relative contributions, detect extras
	for (int bi = theBins.count() - 1; bi >=0 ; --bi) {
		TblDistrBin *b = theBins[bi];
		b->relContrib(b->absContrib()/sum);
		if (b->relContrib() * TheIdxLmt < 1)
			extras.append(b);
		else
			core.append(b);
	}

	// kill gaps in the main index
	theBins.reset();
	while (core.count()) theBins.append(core.pop());

	// create extras if needed
	if (extras.count()) {
		double abs_contrib = 0;
		theExtras = new TblDistr(theName);
		while (extras.count()) {
			TblDistrBin *b = extras.pop();
			abs_contrib += b->absContrib();
			b->owner(0);
			theExtras->addBin(b);
		}
		TblDistrExtras *ebin = new TblDistrExtras(this, theExtras);
		ebin->absContrib(abs_contrib);
		ebin->relContrib(abs_contrib/sum);
		ebin->rndGen(theGen);
		theBins.append(ebin);
	}
}

// must be called after TblDistr::preprocess
void TblDistr::buildSelector() {

	// find smallest acceptable space
	const int adj_lmt = TheIdxLmt - theBins.count(); // for rounding errs
	int space_min = theBins.count();
	for (int bi = 0; bi < theBins.count() && space_min < adj_lmt; ++bi) {
		const TblDistrBin *b = theBins[bi];
		while (!b->acceptable(space_min) && space_min < adj_lmt)
			space_min++;
	}

	// calculate space allocation
	Array<int> spaces(theBins.count());
	int space_tot = 0;
	for (int bk = 0; bk < theBins.count(); ++bk) {
		TblDistrBin *b = theBins[bk];
		const int space = b->nom(space_min);
		Assert(space > 0); // all bins must be represented
		space_tot += space;
		spaces.append(space);
	}

	// space_tot might be only slightly larger than space_min
	Assert(space_tot <= space_min + theBins.count());

	// allocate space
	Assert(space_tot <= TheIdxLmt);
	theBinSelector.stretch(space_tot);
	for (int bj = 0; bj < theBins.count(); ++bj) {
		const int space = spaces[bj];
		
		/*theBins[bj]->print(cerr << here << bj << ". " 
			<< " space: " << "\t " << space);*/

		for (int s = 0; s < space; ++s) {
			Assert(!theBinSelector.full());
			theBinSelector.append(bj);
		}
	}
	Assert(theBinSelector.count() == space_tot);
}

// build bin selector
void TblDistr::make() {
	Assert(theBins.count()); // empty distribution table is prohibited
	Assert(!theBinSelector.count()); // re-compiling is not recommended

	preprocess();
	buildSelector();
	if (theExtras)
		theExtras->make();
}

double TblDistr::totAbsContrib() const {
	double sum = 0;
	for (int bi = 0; bi < theBins.count(); ++bi)
		sum += theBins[bi]->absContrib();
	return sum;
}		

ostream &TblDistr::print(ostream &os, ArgPrinter) const {
	return os << pdfName();
}

ostream &TblDistr::printBins(ostream &os) const {
	for (int bi = 0; bi < theBins.count(); ++bi)
		theBins[bi]->print(os);
	return os;
}


/* IntervalSide */

IntervalSide::IntervalSide(double aPoint, SideType aType): thePoint(aPoint), theType(aType) {
}

bool IntervalSide::operator <=(const IntervalSide &s) const {
	if (thePoint < s.thePoint) // - s.thePoint/1e6)
		return true;
	if (thePoint > s.thePoint) // + s.thePoint/1e6)
		return false;

	if (theType == s.theType) // complete match
		return true;
	if (theType == sup)     // left limit is almost smaller
		return true;
	if (theType == inc && s.theType == inf) // point is smaller than right limit
		return true;

	return false;
}

ostream &IntervalSide::print(ostream &os) const {
	switch (theType) {
		case inf: return os << '(' << thePoint;
		case inc: return os << thePoint;
		case sup: return os << thePoint << ')';
	}
	return os; // make compiler happy
}

/* TblDistrBin */

TblDistrBin::TblDistrBin(): theOwner(0), theAbsContrib(0), theRelContrib(0) {
}

TblDistrBin::~TblDistrBin() {
}

void TblDistrBin::owner(TblDistr *const anOwner) {
	Assert(!anOwner != !theOwner);
	theOwner = anOwner;
}

bool TblDistrBin::covered(const IntervalSide &val) const {
	return left() <= val && val <= right();
}

int TblDistrBin::nom(int denom) const {
	return (int)xceil(theRelContrib*denom, 1); 
}

// acceptable representation using int fraction?
bool TblDistrBin::acceptable(int denom) const {
	const double approx = nom(denom)/(double)denom;
	return fabs(approx - theRelContrib) < theRelContrib/1e5;
}


/* TblDistrPoint */

TblDistrPoint::TblDistrPoint(double aPoint): thePoint(aPoint) {
}

void TblDistrPoint::point(double aPoint) {
	thePoint = aPoint;
}

IntervalSide TblDistrPoint::left() const {
	return IntervalSide(thePoint);
}

IntervalSide TblDistrPoint::right() const {
	return IntervalSide(thePoint);
}

double TblDistrPoint::trial() {
	return thePoint;
}

ostream &TblDistrPoint::print(ostream &os) const {
	return os << "\t " << thePoint << "\t " << (100*theRelContrib) << endl;
}


/* TblDistrIntvl */

TblDistrIntvl::TblDistrIntvl(): theMin(0), theSup(-1) {
}

TblDistrIntvl::TblDistrIntvl(double aMin, double aSup) {
	limits(aMin, aSup);
}

void TblDistrIntvl::limits(double aMin, double aSup) {
	Assert(aMin < aSup);
	theMin = aMin;
	theSup = aSup;
}

IntervalSide TblDistrIntvl::left() const {
	return IntervalSide(theMin);
}

IntervalSide TblDistrIntvl::right() const {
	return IntervalSide(theSup, IntervalSide::sup);
}

double TblDistrIntvl::trial() {
	Assert(theOwner);
	RndGen *const rng = theOwner->RndDistr::rndGen();
	Assert(rng);
	return (*rng)(theMin, theSup);
}

ostream &TblDistrIntvl::print(ostream &os) const {
	return os
		<< "\t " << theMin << ':' << theSup 
		<< "\t " << (100*theRelContrib) << endl;
}

/* TblDistrExtras */

TblDistrExtras::TblDistrExtras(TblDistr *const anOwner, TblDistr *const aDist):
	theDist(aDist) {
	owner(anOwner);
}

TblDistrExtras::~TblDistrExtras() {
	delete theDist;
}

void TblDistrExtras::rndGen(RndGen *aGen) {
	Assert(theDist);
	theDist->rndGen(aGen);
}

// should not be called
IntervalSide TblDistrExtras::left() const {
	Assert(0);
	return IntervalSide(0);
}

// should not be called
IntervalSide TblDistrExtras::right() const {
	Assert(0);
	return IntervalSide(0);
}

double TblDistrExtras::trial() {
	Assert(theDist);
	return theDist->trial();
}

ostream &TblDistrExtras::print(ostream &os) const {
	Assert(theDist);
	return theDist->printBins(os);
}


TblDistr *TblDistr::FromDistrTable(const String &name, Array<double> &probs) {
	TblDistr *td = new TblDistr(name);
	for (int i = 0; i < probs.count(); ++i) {
		if (probs[i] > 0) {
			TblDistrBin *bin = new TblDistrPoint(i);
			bin->absContrib(probs[i]);
			td->addBin(bin);
		}
	}
	td->make();	
	return td;
}
