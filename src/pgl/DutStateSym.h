
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_DUTSTATESYM_H
#define POLYGRAPH__PGL_DUTSTATESYM_H

#include "xstd/Time.h"
#include "pgl/PglRecSym.h"

class BigSize;

// run goal specification
class DutStateSym: public RecSym {
	public:
		static const String TheType;

	public:
		DutStateSym();
		DutStateSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		//bool configured() const;

		Time rptmMin() const;
		Time rptmMax() const;
		bool fillSizeMin(BigSize &size) const;
		bool fillSizeMax(BigSize &size) const;
		bool xactionsMin(int &val) const;
		bool xactionsMax(int &val) const;
		bool repRateMin(double &val) const;
		bool repRateMax(double &val) const;
		bool errorsMin(int &val) const;
		bool errorsMax(int &val) const;
		bool errorRatioMin(double &val) const;
		bool errorRatioMax(double &val) const;
		bool dhrMin(double &val) const;
		bool dhrMax(double &val) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
