
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

bool ContainerBodyIter::pourMiddle() {
	Assert(theContentSize.known()); // formulas below assume that for now
	Assert(theSuffixSize.known()); // formulas below assume that for now
	while (canPour() && middleSizeLeft() > 0) {
		// fill up to next embed tag
		const Size nextSizeGoal = theEmbedCount < theEmbedGoal ?
			Max(theBuiltSize, theEmbedDist*theEmbedCount + theEmbedDist) :
			theContentSize - theSuffixSize;
		if (const Size left = nextSizeGoal - theBuiltSize)
			pourRandom(left);

		// put the embed tag
		if (theEmbedCount < theEmbedGoal) {
			if (!embed())
				break; // not enough buf space to put the tag!
		}
	}

	return true;
}

// writes embedded oid tag into the buffer if there is enough space
// and we are not exceeding content length; return #bytes appended
bool ContainerBodyIter::embed() {
	// some stringstream cannot accept zero-sized buffers
	if (!theBuf->spaceSize())
		return false;

	const ObjId eid = theModel->embedRndOid(theOid, theEmbedCount, theRng);

	ofixedstream os(theBuf->space(), theBuf->spaceSize());
	os << "<embed src=\"";
	Oid2UrlPath(eid, os);
	os << "\">";
	const bool fit = os != 0;
	const Size sz = Size(os.tellp());

	// give up if the tag does not fit into content-length left
	// give up if the tag is bigger than the buffer
	const Size todo = middleSizeLeft();
	if (todo < sz || (!fit && theBuf->empty()))
		return pourRandom(todo);

	if (fit) {
		theBuf->appended(sz);
		Should(*(theBuf->space()-1) == '>'); // paranoid
		theBuiltSize += sz;
		theEmbedCount++;
		return true;
	}

	// the tag will probably fit, given more buffer space
	return false;
}

ContainerBodyIter *ContainerBodyIter::clone() const {
	return new ContainerBodyIter(*this);
}

void ContainerBodyIter::calcContentSize() const {
	Should(false);
}
