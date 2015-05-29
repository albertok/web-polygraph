
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "logextractors/LogIter.h"

LogIter::LogIter(ILog *aLog): theLog(0) {
	if (aLog)
		start(aLog);
}

LogIter &LogIter::start(ILog *aLog) {
	theLog = aLog;
	thePx = LogEntryPx();
	sync();
	return *this;
}

bool LogIter::eof() const {
	return !theLog || !*theLog || !thePx;
}

LogIter::operator void *() const {
	return !eof() ? (void*)(-1) : 0;
}

LogIter &LogIter::operator ++() {
	Assert(theLog);
	theLog->endEntry();
	sync();
	return *this;
}

const LogEntryPx &LogIter::pos() const {
	Assert(!eof());
	return thePx;
}

void LogIter::sync() {
	thePx = theLog->begEntry();
	if (eof())
		thePx = LogEntryPx();
}
