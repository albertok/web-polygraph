
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"
#include "xstd/h/iostream.h"

#include "xstd/Rnd.h"
#include "pgl/popDistrs.h"


/* UnifPopDistr */

int64_t UnifPopDistr::choose(RndGen &rng, int64_t lastOid) {
	return 1 + rng(static_cast<int64_t>(0), lastOid);
}

ostream &UnifPopDistr::print(ostream &os) const {
	return os << "popUnif()";
}


/* ZipfPopDistr */

ZipfPopDistr::ZipfPopDistr(double aSkew): theSkew(aSkew) {
}

int64_t ZipfPopDistr::choose(RndGen &rng, int64_t lastOid) {
	const double rn = rng();
	return 1 + lastOid - static_cast<int64_t>(pow(lastOid+1, pow(rn,theSkew)));
}

ostream &ZipfPopDistr::print(ostream &os) const {
	return os << "popZipf(" << theSkew << ')';
}
