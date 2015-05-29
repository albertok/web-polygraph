
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_LOGOBJ_H
#define POLYGRAPH__BASE_LOGOBJ_H

// an optional parent of logged objects

#include "base/OLog.h"
#include "base/ILog.h"

struct LogObj {
	virtual ~LogObj() {}
	virtual OLog &store(OLog &ol) const = 0;
	virtual ILog &load(ILog &il) = 0;
};

inline OLog &operator <<(OLog &log, const LogObj &o) { o.store(log); return log; }
inline ILog &operator >>(ILog &log, LogObj &o) { o.load(log); return log; }


#endif
