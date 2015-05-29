
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_SIZEHISTFIG_H
#define POLYGRAPH__LOGANALYZERS_SIZEHISTFIG_H

#include "loganalyzers/TmSzHistFig.h"


// creates siae distribution figure based on phase stats
class SizeHistFig: public TmSzHistFig {
	public:
		SizeHistFig();

	protected:
		virtual const Histogram *extractHist(const Stex *stex, const PhaseInfo &info) const;
};

#endif
