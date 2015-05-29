
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/Histogram.h"
#include "base/TmSzHistStat.h"
#include "loganalyzers/Stex.h"
#include "loganalyzers/RptmHistFig.h"


RptmHistFig::RptmHistFig(): TmSzHistFig("msec") {
}

const Histogram *RptmHistFig::extractHist(const Stex *stex, const PhaseInfo &info) const {
	const TmSzHistStat *h = stex->hist(info);
	return h ? &h->time() : 0;
}
