
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "xstd/gadgets.h"
#include "runtime/MessageSize.h"

MessageSize::MessageSize() {
	reset();
}

void MessageSize::reset() {
	theExpSize = Size();
	theActSize = 0;
	theHdrSize = Size();
	isExpectingWhatParsed = false;
	hasExpectedBody = false;
}

Size MessageSize::expectToGet(Size availSize) const {
	if (expected().known())
		return Min(availSize, theExpSize - theActSize);
	else
		return availSize;
}

void MessageSize::store(OLog &ol) const {
	ol << theExpSize << theActSize; // not storing isExpectingWhatParsed
}

void MessageSize::load(ILog &il) {
	il >> theExpSize >> theActSize; // not loading isExpectingWhatParsed
}
