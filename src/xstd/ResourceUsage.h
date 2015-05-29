
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_RESOURCEUSAGE_H
#define POLYGRAPH__XSTD_RESOURCEUSAGE_H

#include "xstd/Time.h"
#include "xstd/String.h"
#include "xstd/h/iosfwd.h"
#include "xstd/h/sys/resource.h"

// a wrapper arround highly unportable "getrusage()" system call
class ResourceUsage: protected rusage {
	public:
		static ResourceUsage Current(); // current rusage

	public:
		ResourceUsage();

		Time sysTime() const;
		Time userTime() const;
		Time cpuTime() const;
		Size maxResSize() const;
		long physPageFaults() const;

		bool update(); // syncronize with current usage

		void report(ostream &os, const String &pfx) const;
};


inline
ostream &operator <<(ostream &os, const ResourceUsage &ru) {
	ru.report(os, "");
	return os;
}

#endif
