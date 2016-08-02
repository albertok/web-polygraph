
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_POPDISTRS_H
#define POLYGRAPH__PGL_POPDISTRS_H

#include "pgl/PopDistr.h"

// uniform
class UnifPopDistr: public PopDistr {
	public:
		virtual int64_t choose(RndGen &rng, int64_t lastName);
		virtual ostream &print(ostream &os) const;
};

// zipf power law emulation
class ZipfPopDistr: public PopDistr {
	public:
		ZipfPopDistr(double aSkew = 1);

		virtual int64_t choose(RndGen &rng, int64_t lastName);
		virtual ostream &print(ostream &os) const;

	protected:
		double theSkew;
};

#endif
