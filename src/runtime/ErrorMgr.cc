
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ErrorRec.h"
#include "runtime/LogComment.h"
#include "runtime/SharedOpts.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/ErrorMgr.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"

ErrorMgr TheErrorMgr;


ErrorMgr::ErrorMgr() {
}

void ErrorMgr::printError(const SrcLoc &loc, ErrorRec *rec, int logCat) {
	Assert(rec);
	const Counter totCnt = TheStatPhaseMgr->errors(logCat).count();
	Comment(0) << loc << "error: " << rec->count() << '/' << totCnt 
		<< ' ' << *rec << endc;
	rec->noteReport(TheOpts.theErrorTout);
}

bool ErrorMgr::reportError(const SrcLoc &loc, const Error &e, int logCat) {
	Assert(logCat > 0);
	ErrorRec *rec = 0;
	bool needRep = false;
	if ((needRep = TheStatPhaseMgr->errors(logCat).record(e, rec)))
		printError(loc, rec, logCat); // print asap
	Broadcast(TheErrChannel, e);
	return needRep;
}

void ErrorMgr::fatalError(const SrcLoc &loc, const Error &e, int logCat) {
	Assert(logCat > 0);
	if (e != errOther) {
		// fatal errors are not broadcasted to quit asap
		ErrorRec *rec = 0;
		TheStatPhaseMgr->errors(logCat).record(e, rec);
		printError(loc, rec, logCat);
	}
	Comment(0) << "last error was fatal; terminating..." << endc;
	exit(e.no());
}
