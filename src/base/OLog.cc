
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#ifdef HAVE_UNISTD_H
#include "xstd/h/os_std.h"
#endif

#include "xstd/h/string.h"

#include "base/OLog.h"
#include "xstd/gadgets.h"


OLog::OLog(): theStream(0), theBuf(0), theEntry(0), theEntryTag(-1) {
	theCapacity = Size::KB(64); // default
	theBuf = new char[theCapacity];
	theSize = 0;
	thePos = 0;
}

OLog::~OLog() {
	if (theStream)
		close();
	delete[] theBuf;
}

void OLog::stream(const String &aName, ostream *aStream) {
	Assert(!theStream && aStream);
	theName = aName;
	theStream = aStream;

	putHeader();
}

void OLog::capacity(Size aCap) {
	Assert(theCapacity);
	Assert(aCap >= theCapacity); // cannot shrink

	if (aCap > theCapacity) {
		theCapacity = aCap;

		// allocate new chunk and copy
		char *buf = new char[theCapacity];
		if (theSize)
			memcpy(buf, theBuf, theSize);
		if (theEntry)
			theEntry = buf + (theEntry - theBuf);
		delete[] theBuf;
		theBuf = buf;
	}
}

void OLog::resize(Size minCap) {
	Assert(minCap >= theCapacity); // cannot shrink

	// exponential growth
	Size newCap = theCapacity;
	while (newCap < minCap)
		newCap *= 2;

	capacity(newCap);
}

void OLog::close() {
	if (theStream) {

		// abort unfinished entry, if any
		if (theEntry) {
			const Size readySize = (Size)(theEntry - theBuf);
			thePos -= theSize - readySize;
			theSize = readySize;
			theEntry = 0;
		}

		putTrailer();
		flush();
		delete theStream;
		theStream = 0;
	}
}

void OLog::flush(Size maxSize) {
	if (!theStream)
		return;

	// do not flush current entry if any; its size is yet unknown
	const Size readySz = Min(maxSize,
		theEntry ? (Size)(theEntry - theBuf) : theSize);
	if (readySz > 0) {
		Should(theStream->write(theBuf, readySz));
		if (theSize > readySz) {
			// move leftovers to the beginning of a buffer
			theSize -= readySz;
			memmove(theBuf, theBuf + readySz, theSize);
		} else {
			Assert(theSize == readySz); // wrote everything
			theSize = 0;
		}
		if (theEntry)
			theEntry -= readySz;
	}
}

void OLog::overflow(const void *buf, Size size) {
	// buffer what fits
	int space = theCapacity - theSize;
	if (space > 0) {
		put(buf, space);
		buf = ((const char*)buf) + space;
		size -= space;
	}

	flush();

	// grow if does not fit
	space = theCapacity - theSize;
	if (space < size)
		resize(theSize + size);

	Assert(theSize + size <= theCapacity);
	put(buf, size);
}

void OLog::begEntry(int tag) {
	Assert(tag > 0);
	Assert(!theEntry);

	theEntry = theBuf + theSize;
	theEntryTag = tag;
	puti(0);		// size placeholder
	puti(tag);
}

void OLog::endEntry() {
	Assert(theEntry);
	Assert(theEntryTag > 0);
	const int x = htonl((theBuf + theSize) - theEntry);
	memcpy(theEntry, &x, sizeof(x)); // record actual entry size

	// update directory if needed
	if (theDir.count() > theEntryTag) { // old tag
		if (!theDir[theEntryTag])
			theDir[theEntryTag] = thePos;
		else
		if (theDir[theEntryTag] > 0)
			theDir[theEntryTag] = -theDir[theEntryTag];
	} else {
		theDir.put(thePos, theEntryTag);
	}

	theEntryTag = -1;
	theEntry = 0;
}

void OLog::putHeader() {
	puti(21); // current version
	puti(21); // required min version to support
	puti(0);  // size of extra headers
}

void OLog::putTrailer() {
	puti(0); // mark end-of-log
	puti(0); // reserved
	const Size dirPos = thePos;
	(*this) << theDir;
	puti(thePos - dirPos); // dir size
}

void OLog::puti(const int *xs, int count) {
	puti(count);
	for (int i = 0; i < count; ++i)
		puti(xs[i]);
}
