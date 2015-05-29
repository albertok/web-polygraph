
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGEXTRACTORS_LOGCATFILTER_H
#define POLYGRAPH__LOGEXTRACTORS_LOGCATFILTER_H

#include "xstd/Array.h"

class LogEntryPx;

class LogCatFilter {
	public:
		LogCatFilter(int aCat = -1);

		void logCat(int aCat);

		// true for lgcAll and for the prime cat
		bool passed(const LogEntryPx &px);

	protected:
		void addCat(int cat);

	protected:
		Array<bool> theActiveCats; // active categories
		int theCat;                // "prime" category
		
};

#endif
