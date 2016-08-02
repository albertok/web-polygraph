
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "xstd/Rnd.h"
#include "base/RndPermut.h"
#include "runtime/HotSet.h"


HotSet::HotSet(): thePos(0) {
}

void HotSet::reset() {
	thePos = 0;
}

void HotSet::syncPos(const Counter size, const Counter wss) {
	if (size && wss != 0) {
		if (wss < 0) {
			// the best we can do w/o storing size-at-change?
			thePos = size/2; 
		} else
		if (thePos <= size - wss) {
			// random position within working set
			Assert(wss <= size);
			RndGen rng(LclPermut(thePos, rndHotSetPos));
			thePos = size - rng(static_cast<Counter>(0), wss);
		}
	}
	Assert(0 <= thePos && thePos <= size);
}
