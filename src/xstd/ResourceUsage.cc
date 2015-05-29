
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/string.h"
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Error.h"
#include "xstd/ResourceUsage.h"


ResourceUsage ResourceUsage::Current() {
	ResourceUsage ru;
	ru.update();
	return ru;
}

ResourceUsage::ResourceUsage() {
	memset(this, 0, sizeof(*this));
}

Time ResourceUsage::sysTime() const {
	return Time(ru_stime);
}

Time ResourceUsage::userTime() const {
	return Time(ru_utime);
}

Time ResourceUsage::cpuTime() const {
	return userTime() + sysTime();
}

Size ResourceUsage::maxResSize() const {
	return ::MaxRss2Size(ru_maxrss);
}

long ResourceUsage::physPageFaults() const {
	return ru_majflt;
}

bool ResourceUsage::update() {
#if defined(HAVE_GETRUSAGE) && defined(RUSAGE_SELF)
	return ::getrusage(RUSAGE_SELF, this) == 0;
#else
	Error::Last(EOPNOTSUPP);
	return false;
#endif
}

void ResourceUsage::report(ostream &os, const String &pfx) const {
	os <<
		pfx << "CPU Usage: " 
			<< sysTime() << " sys + " 
			<< userTime() << " user = " 
			<< cpuTime()
			<< endl;
	if (Size sz = maxResSize())
		os << pfx << "Maximum Resident Size: " << sz << endl;

	os 
		<< pfx << "Page faults with physical i/o: " << physPageFaults() << endl
		;
}
