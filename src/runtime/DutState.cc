
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "base/StatIntvlRec.h"
#include "pgl/DutStateSym.h"
#include "runtime/DutState.h"


DutState::DutState() {
}

void DutState::configure(const DutStateSym &cfg) {
	const Time rptmMin = cfg.rptmMin();
	if (rptmMin >= 0)
		rptm.min(rptmMin);
	const Time rptmMax = cfg.rptmMax();
	if (rptmMax >= 0)
		rptm.max(rptmMax);

	BigSize fillSizeMin;
	if (cfg.fillSizeMin(fillSizeMin))
		fillSize.min(fillSizeMin);
	BigSize fillSizeMax;
	if (cfg.fillSizeMax(fillSizeMax))
		fillSize.max(fillSizeMax);

	int xactionsMin;
	if (cfg.xactionsMin(xactionsMin))
		xactions.min(xactionsMin);
	int xactionsMax;
	if (cfg.xactionsMax(xactionsMax))
		xactions.max(xactionsMax);

	double repRateMin;
	if (cfg.repRateMin(repRateMin))
		repRate.min(repRateMin);
	double repRateMax;
	if (cfg.repRateMax(repRateMax))
		repRate.max(repRateMax);

	int errorsMin;
	if (cfg.errorsMin(errorsMin))
		errors.min(errorsMin);
	int errorsMax;
	if (cfg.errorsMax(errorsMax))
		errors.max(errorsMax);

	double errorRatioMin;
	if (cfg.errorRatioMin(errorRatioMin))
		errorRatio.min(errorRatioMin);
	double errorRatioMax;
	if (cfg.errorRatioMax(errorRatioMax))
		errorRatio.max(errorRatioMax);

	double dhrMin;
	if (cfg.dhrMin(dhrMin))
		dhr.min(dhrMin);
	double dhrMax;
	if (cfg.dhrMax(dhrMax))
		dhr.max(dhrMax);
}

bool DutState::reached(const StatIntvlRec &stats) const {
	if (!fillSize.contains(stats.totFillSize()))
		return false;

	if (!xactions.contains(stats.xactCnt()))
		return false;

	if (!errors.contains(stats.theXactErrCnt))
		return false;

	// keep this check in sync with checks below
	if (!stats.xactCnt())
		return false; // no valid values to check following conditions 

	if (!rptm.contains(Time::Msec((int)stats.repTime().mean())))
		return false;

	if (!repRate.contains(stats.repRate()))
		return false;

	if (!errorRatio.contains(stats.errRatio()))
		return false;

	if (!dhr.contains(stats.theRealHR.dhr()))
		return false;

	return true;
}

ostream &DutState::print(ostream &os, const String &pfx) const {
	int knownCount = 0;

	if (rptm.known() && ++knownCount)
		os << pfx << "rptm:       " << rptm << endl;
	if (fillSize.known() && ++knownCount)
		os << pfx << "fillSize:   " << fillSize << endl;
	if (xactions.known() && ++knownCount)
		os << pfx << "xactions:   " << xactions << endl;
	if (repRate.known() && ++knownCount)
		os << pfx << "repRate:    " << repRate << endl;
	if (errors.known() && ++knownCount)
		os << pfx << "errors:     " << errors << endl;
	if (errorRatio.known() && ++knownCount)
		os << pfx << "errorRatio: " << errorRatio << endl;
	if (dhr.known() && ++knownCount)
		os << pfx << "dhr:        " << dhr << endl;

	if (knownCount == 0)
		os << pfx << "undef()" << endl;

	return os;
}
