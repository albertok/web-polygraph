
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_DUTSTATE_H
#define POLYGRAPH__RUNTIME_DUTSTATE_H

#include "xstd/Time.h"
#include "xstd/BigSize.h"
#include "runtime/MeasRange.h"

class DutStateSym;
class StatIntvlRec;

// describes desired DUT state
class DutState {
	public:
		DutState();

		void configure(const DutStateSym &cfg);

		bool reached(const StatIntvlRec &stats) const;

		ostream &print(ostream &os, const String &pfx) const;

	public:
		MeasRange<Time> rptm;
		MeasRange<BigSize> fillSize;
		MeasRange<int> xactions;
		MeasRange<double> repRate;
		MeasRange<int> errors;
		MeasRange<double> errorRatio;
		MeasRange<double> dhr;
};

#endif
