
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGEXTRACTORS_LOGITER_H
#define POLYGRAPH__LOGEXTRACTORS_LOGITER_H

#include "base/ILog.h"

// iterates ILog
class LogIter {
	public:
		LogIter(ILog *aLog = 0);

		LogIter &start(ILog *aLog);

		operator void *() const;
		LogIter &operator ++();

		ILog &log() { return *theLog; }

		const LogEntryPx &operator *() const { return pos(); }
		const LogEntryPx *operator ->() const { return &pos(); }
		const LogEntryPx &pos() const;  // current position

	protected:
		void sync();
		bool eof() const;

	protected:
		ILog *theLog;
		LogEntryPx thePx;
};

#endif
