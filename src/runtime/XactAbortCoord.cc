
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/XactAbortCoord.h"


XactAbortCoord::XactAbortCoord(): theWhetherState(0), theWhereState(0) {
}

void XactAbortCoord::configure(const RndGen::Seed whether, const RndGen::Seed where) {
	theWhetherState = whether;
	theWhereState = where;
}

void XactAbortCoord::reset() {
	theWhetherState = 0;
	theWhereState = 0;
}

Size XactAbortCoord::pos(Size hdrSize, Size bodySize) const {
	if (bodySize > 0) {
		RndGen rng;
		rng.state(theWhereState);
		const Size where = rng(0, (int)bodySize);
		return rng.state() == theWhetherState ? hdrSize + where : Size(-1);
	} else {
		return -1;
	}
}
