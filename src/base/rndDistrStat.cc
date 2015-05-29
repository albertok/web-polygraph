
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>

#include "xstd/Rnd.h"
#include "base/AggrStat.h"
#include "xstd/gadgets.h"

AggrStat RndDistrStat(RndDistr *d, double scale, int sampleSize) {
	Assert(d);
	AggrStat s;
	while (sampleSize-- > 0) {
		const double t = scale * d->trial();
		s.record((int)MiniMax((double)INT_MIN, t, (double)INT_MAX));
	}
	return s;
}
