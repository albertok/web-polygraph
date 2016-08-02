
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "xstd/h/iostream.h"
#include "xstd/h/string.h"
#include "xstd/CpuAffinitySet.h"

static std::ostream &operator <<(std::ostream &os, const cpu_set_t &cpuSet);


CpuAffinitySet::CpuAffinitySet() {
	CPU_ZERO(&raw);
}

bool CpuAffinitySet::apply(ostream &err) {
	Must(CPU_COUNT(&raw) > 0);

	cpu_set_t origCpuSet;
	CPU_ZERO(&origCpuSet);
	if (sched_getaffinity(0, sizeof(origCpuSet), &origCpuSet) != 0) {
		err << "failed to get original CPU affinity: " << Error::Last();
        return false;
	}

	// CPU_AND to be able to distinguish EINVAL due to external restrictions
	// from other EINVAL errors after calling sched_setaffinity().
	cpu_set_t allowedCpuSet;
	memcpy(&allowedCpuSet, &raw, sizeof(allowedCpuSet));
	CPU_AND(&allowedCpuSet, &allowedCpuSet, &origCpuSet);
	if (CPU_COUNT(&allowedCpuSet) <= 0) {
		err << "requested CPU affinity is incompatible with preexisting restrictions" << std::endl <<
			"requested affinity: " << raw << std::endl <<
			"existing affinity:  " << origCpuSet;
		return false;
	}

	if (sched_setaffinity(0, sizeof(allowedCpuSet), &allowedCpuSet) != 0) {
		err << "failed to set CPU affinity: " << Error::Last() << std::endl <<
			"CPU affinity (after adjustments) was: " << allowedCpuSet;
		return false;
	}

	cpu_set_t resultingCpuSet;
	CPU_ZERO(&resultingCpuSet);
	if (sched_getaffinity(0, sizeof(resultingCpuSet), &resultingCpuSet) != 0) {
		err << "failed to get changed CPU affinity" << Error::Last();
		return false;
	}

	if (!CPU_EQUAL(&raw, &resultingCpuSet)) {
		err << "resulting/set CPU affinity: " << resultingCpuSet << std::endl <<
			"differs from the configured CPU affinity: " << raw;
		return true; // treat this as a warning, not error
	}

	return true;
}

void CpuAffinitySet::reset(const int coreId, const bool turnOn) {
	if (turnOn)
		CPU_SET(coreId, &raw);
	else
		CPU_CLR(coreId, &raw);
}

std::ostream &CpuAffinitySet::print(std::ostream &os) const {
	return os << raw;
}

static
std::ostream &operator <<(std::ostream &os, const cpu_set_t &cpuSet) {
	const int count = CPU_COUNT(&cpuSet);
	os << '[';
	for (int printed = 0, i = 0; i < CPU_SETSIZE && printed < count; ++i) {
		if (CPU_ISSET(i, &cpuSet)) {
			os << i;
			if (++printed < count)
				os << ',';
		}
	}
	os << ']';
	return os;
}
