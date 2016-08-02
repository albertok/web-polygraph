
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "logextractors/LogCatFilter.h"

#include "base/polyLogCats.h"


LogCatFilter::LogCatFilter(int aCat): theCat(lgcAll) {
	theActiveCats.resize(lgcEnd);
	if (aCat >= 0)
		logCat(aCat);
}

void LogCatFilter::logCat(int aCat) {
	theCat = aCat;
}

bool LogCatFilter::passed(const LogEntryPx &px) {

	if (!theActiveCats[px.theCat])
		addCat(px.theCat);

	return
		theCat == lgcAll ||
		px.theCat == lgcAll ||
		px.theCat == theCat;
}

void LogCatFilter::addCat(int cat) {
	// find the first active category, excluding lgcAll
	int firstCat = lgcAll;
	for (int i = 1; i < theActiveCats.count() && firstCat == lgcAll; ++i) {
		if (theActiveCats[i])
			firstCat = i;
	}

	if (firstCat > lgcAll && theCat == lgcAll) {
		cerr << "warning: log(s) have info from client and server sides,"
			<< " and no specific side was specified; "
			<< " assuming `"
				<< (firstCat == lgcCltSide ? "clt" : "srv")
				<< "' side"
			<< endl;
		theCat = firstCat;
	}

	theActiveCats[cat] = true;
}
