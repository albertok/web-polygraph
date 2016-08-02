
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_CPU_AFFINITY_SET_H
#define POLYGRAPH__XSTD_CPU_AFFINITY_SET_H

#include "xstd/h/sched.h"
#include "xstd/h/iosfwd.h"

// cpu affinity management for a single process
class CpuAffinitySet {
public:
	CpuAffinitySet();

	bool apply(ostream &err); // set CPU affinity for this process
	void reset(const int coreId, const bool turnOn); // set CPU affinity bit

	std::ostream &print(std::ostream &os) const;

private:
	cpu_set_t raw; // low-level storage
};

std::ostream &operator <<(std::ostream &os, const CpuAffinitySet &cpuSet);

#endif
