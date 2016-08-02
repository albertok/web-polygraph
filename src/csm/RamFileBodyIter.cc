
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "runtime/IOBuf.h"
#include "csm/RamFileBodyIter.h"
#include "xstd/gadgets.h"


RamFileBodyIter::RamFileBodyIter(): theOff(0) {
}

void RamFileBodyIter::file(const RamFile &aFile) {
	theFile = aFile;
}

void RamFileBodyIter::start(WrBuf *aBuf) {
	BodyIter::start(aBuf);
	Assert(!theOff);
}

bool RamFileBodyIter::pourMiddle() {
	const Size fileSize = theFile.body.len();
	const char *const fileData = theFile.body.data();

	while (canPour() && middleSizeLeft()) {
		if (theOff >= fileSize)
			theOff = 0;

		const Size pour = Min(theBuf->spaceSize(), middleSizeLeft());
		const Size dataSize = fileSize - theOff;
		const Size size = Min(pour, dataSize);

		theBuf->append(fileData + theOff, size);
		theBuiltSize += size;
		theOff += size;
	}
	return true;
}

RamFileBodyIter *RamFileBodyIter::clone() const {
	return new RamFileBodyIter(*this);
}

void RamFileBodyIter::calcContentSize() const {
	ShouldUs(false);
}
