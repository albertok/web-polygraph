
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/gadgets.h"
#include "runtime/IOBuf.h"
#include "base/ObjId.h"
#include "csm/EmbedContMdl.h"
#include "csm/ContainerBodyIter.h"
#include "csm/oid2Url.h"


ContainerBodyIter::ContainerBodyIter():
	theModel(0), theEmbedGoal(-1), theEmbedCount(0) {
}

void ContainerBodyIter::embedContModel(EmbedContMdl *aModel) {
	Assert(!theModel && aModel);
	theModel = aModel;
}

void ContainerBodyIter::start(WrBuf *aBuf) {
	BodyIter::start(aBuf);
	Assert(theModel);
	Assert(theEmbedCount == 0);
	theEmbedGoal = theModel->embedGoal(theRng);
	Assert(theEmbedGoal >= 0);
	Assert(theContentSize.known());
	theEmbedDist = theContentSize / (theEmbedGoal+1);
}

bool ContainerBodyIter::pourBody() {
	Assert(theContentSize.known()); // formulas below assume that for now
	while (canPour()) {
		// fill up to next embed tag
		const Size nextSizeGoal = theEmbedCount < theEmbedGoal ?
			Max(theBuiltSize, theEmbedDist*theEmbedCount + theEmbedDist) :
			theContentSize;
		if (const Size left = nextSizeGoal - theBuiltSize) {
			theBuiltSize += theBuf->appendRndUpTo(
				IOBuf::RandomOffset(offSeed(), theBuiltSize), left);
		}

		// put the embed tag
		if (theEmbedCount < theEmbedGoal) {
			if (const int sz = embed())
				theBuiltSize += sz;
			else
				break; // not enough buf space to put the tag!
		}
	}

	return true;
}

// writes embedded oid tag into the buffer if there is enough space
// and we are not exceeding content length; return #bytes appended
Size ContainerBodyIter::embed() {
	// some stringstream cannot accept zero-sized buffers
	if (!theBuf->spaceSize())
		return 0;

	const ObjId eid = theModel->embedRndOid(theOid, theEmbedCount, theRng);

	ofixedstream os(theBuf->space(), theBuf->spaceSize());
	os << "<embed src=\"";
	Oid2UrlPath(eid, os);
	os << "\">";
	const bool fit = os != 0;
	const Size sz = Size(os.tellp());

	// give up if the tag does not fit into content-length left
	// give up if the tag is bigger than the buffer
	if (sizeLeft() < sz || (!fit && theBuf->empty())) {
		return theBuf->appendRndUpTo(
			IOBuf::RandomOffset(offSeed(), theBuiltSize), sizeLeft());
	}

	if (fit) {
		theBuf->appended(sz);
		Should(*(theBuf->space()-1) == '>'); // paranoid
		theEmbedCount++;
		return sz;
	}

	// the tag will probably fit, given more buffer space
	return 0;
}

ContainerBodyIter *ContainerBodyIter::clone() const {
	return new ContainerBodyIter(*this);
}

void ContainerBodyIter::calcContentSize() const {
	Should(false);
}
