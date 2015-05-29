
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_TBLDISTR_H
#define POLYGRAPH__XSTD_TBLDISTR_H

#include "xstd/Array.h"
#include "xstd/String.h"
#include "xstd/Rnd.h"

class TblDistrBin;
class IntervalSide;

// "tabular" distribution: 
// a table with probabilities for each entry/bin
class TblDistr: public RndDistr {
	public:
		static TblDistr *FromDistrTable(const String &name, Array<double> &probs);

	public:
		TblDistr(const String &aName);
		~TblDistr(); // destroys bins

		virtual const char *pdfName() const { return theName.cstr(); }

		// these are impossible or hard to compute
		virtual double mean() const { return -1; }
		virtual double sdev() const { return 0; }

		virtual void rndGen(RndGen *aGen);

		virtual double trial();

		void addBin(TblDistrBin *bin);
		bool overlapping(const TblDistrBin *bin) const;

		int binCount() const; // number of bins
		int idxCount() const; // number of selector entries
		int idxDepth() const; // index tree hight

		void make(); // must be called before first trial

		virtual ostream &print(ostream &os, ArgPrinter p = &RndDistr_DefArgPrinter) const;
		ostream &printBins(ostream &os) const;

	protected:
		bool covered(const IntervalSide &val) const;
		double totAbsContrib() const;
		void preprocess();
		void buildSelector();

	protected:
		static int TheIdxLmt; // maximum number of entries in selector

	protected:
		Array<TblDistrBin*> theBins;
		Array<short> theBinSelector;
		TblDistr *theExtras;
		String theName;
};

// a left or right "side" of an open or closed interval 
class IntervalSide {
	public:
		enum SideType { inf = -1, inc = 0, sup = +1 };

	public:
		IntervalSide(double aPoint, SideType aType = inc);

		bool operator <=(const IntervalSide &s) const;
		ostream &print(ostream &os) const;

	protected:
		double thePoint;
		SideType theType;
};

inline
ostream &operator <<(ostream &os, const IntervalSide &s) { return s.print(os); }


// a bin for a tabular distribution
class TblDistrBin {
	public:
		TblDistrBin();
		virtual ~TblDistrBin();

		void owner(TblDistr *const anOwner);

		double absContrib() const { return theAbsContrib; }
		double relContrib() const { return theRelContrib; }
		bool covered(const IntervalSide &val) const;

		// acceptable representation using int fraction?
		bool acceptable(int denom) const;
		int nom(int denom) const;

		void absContrib(double aContr) { theAbsContrib = aContr; }
		void relContrib(double aContr) { theRelContrib = aContr; }

		virtual IntervalSide left() const = 0;
		virtual IntervalSide right() const = 0;

		virtual double trial() = 0;
		virtual ostream &print(ostream &os) const = 0;

	protected:
		TblDistr *theOwner; // TblDistr this bin belongs to
		double theAbsContrib; // absolute contribution set by owner
		double theRelContrib; // relative contribution set by TblDistr
};

// a single-point bin
class TblDistrPoint: public TblDistrBin {
	public:
		TblDistrPoint(double aPoint = -1);

		void point(double aPoint);

		virtual IntervalSide left() const;
		virtual IntervalSide right() const;

		virtual double trial();
		virtual ostream &print(ostream &os) const;

	protected:
		double thePoint;
};

// a [min, sup) interval bin
class TblDistrIntvl: public TblDistrBin {
	public:
		TblDistrIntvl();
		TblDistrIntvl(double aMin, double aSup);

		void limits(double aMin, double aSup);

		virtual IntervalSide left() const;
		virtual IntervalSide right() const;

		virtual double trial();
		virtual ostream &print(ostream &os) const;

	protected:
		double theMin;
		double theSup;
};

#endif
