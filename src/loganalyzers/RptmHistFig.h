
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_RPTMHISTFIG_H
#define POLYGRAPH__LOGANALYZERS_RPTMHISTFIG_H

#include "loganalyzers/TmSzHistFig.h"


// creates response time distribution figure based on phase stats
class RptmHistFig: public TmSzHistFig {
	public:
		RptmHistFig();

	protected:
		virtual const Histogram *extractHist(const Stex *stex, const PhaseInfo &info) const;
};

#endif
