
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_RNDDISTRSTAT_H
#define POLYGRAPH__BASE_RNDDISTRSTAT_H

#include "base/AggrStat.h"

// some distributions do not have a known mean/dev or we do not know how to
// calculate them; this function samples a given [arbitrary] distribution.

// note1: this function is slow.

// note2: AggrStat operates on integers, RndDistr::trial() returns doubles;
// if needed, use scale parameter, but do not forget to sacle back!

extern AggrStat RndDistrStat(RndDistr *d, double scale = 1, int sampleSize = 10000);

#endif
