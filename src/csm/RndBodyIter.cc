
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/IOBuf.h"
#include "base/ObjId.h"
#include "csm/RndBodyIter.h"

#include "xstd/gadgets.h"

bool RndBodyIter::pourMiddle() {
	pourRandom(middleSizeLeft());
	return true;
}

RndBodyIter *RndBodyIter::clone() const {
	return new RndBodyIter(*this);
}

void RndBodyIter::calcContentSize() const {
	Should(false);
}
