
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_ERRORMGR_H
#define POLYGRAPH__RUNTIME_ERRORMGR_H

#include "xstd/Error.h"
#include "xstd/Array.h"
#include "xstd/SrcLoc.h"
#include "base/ErrorStat.h"

// dumps and broadcasts errors
// use global wrappers below for error notification
class ErrorMgr {
	public:
		ErrorMgr();

		// do not to call these directly
		// use global wrappers below instead
		bool reportError(const SrcLoc &loc, const Error &e, int logCat);
		void fatalError(const SrcLoc &loc, const Error &e, int logCat);

	protected:
		void printError(const SrcLoc &loc, ErrorRec *rec, int logCat);
};

extern ErrorMgr TheErrorMgr;


/* macros to auto-supply source location */

// count, maybe print a std explanation, return true iff did print
// user may want to provide extra information if returned true
#define ReportError(e) TheErrorMgr.reportError(Here, (e), this->logCat())
#define ReportError2(e, logCat) TheErrorMgr.reportError(Here, (e), (logCat))

// count, print a std explanation, exit
// user must print any additional info before calling FatalErrorLoc
#define FatalError(e) TheErrorMgr.fatalError(Here, (e), this->logCat())
#define FatalError2(e, logCat) TheErrorMgr.fatalError(Here, (e), (logCat))


#endif
