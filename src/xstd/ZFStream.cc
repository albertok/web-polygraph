
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2012 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/Assert.h"
#include "xstd/String.h"
#include "xstd/ZFStream.h"
#include "xstd/gadgets.h"
#include "xstd/h/string.h"


/* ZFStream */

template <class Stream, class Encoder>
zlib::FStream<Stream, Encoder>::FStream(Stream &aStream): theStream(aStream),
	theInCapacity(0), theInBuf(0), theOutCapacity(0), theOutBuf(0),
	theInSize(0), thePos(0), theEncodeError(!zlib::Supported) {
}

template <class Stream, class Encoder>
void zlib::FStream<Stream, Encoder>::init(const int anInCapacity, const int anOutCapacity) {
	Must(!theInCapacity && !theInBuf && !theOutCapacity && !theOutBuf);
	theInCapacity = anInCapacity;
	theInBuf = new char[theInCapacity];
	theOutCapacity = anOutCapacity;
	theOutBuf = new char[theOutCapacity];
}


/* OFStream */

zlib::OFStream::OFStream(ostream &stream): FStream<ostream, Deflator>(stream) {
	thePos = theStream.tellp();
	const int capacity = 64 * 1024;
	init(capacity, theEncoder.bound(capacity));
}

bool zlib::OFStream::write(const char *buf, int len) {
	Must(!theInSize);

	if (fail())
		return false;

	while (len > 0 || theInSize > 0) {
		const int size = Min(len, theInCapacity - theInSize);
		if (size > 0) {
			memmove(theInBuf + theInSize, buf, size);
			len -= size;
			theInSize += size;
			thePos += size;
			buf += size;
		}
		if (!deflateAndWrite(!len))
			return false;
	}

	return true;
}

bool zlib::OFStream::deflateAndWrite(const bool doSyncFlush) {
	Size spaceDelta = 0;
	Size dataDelta = 0;
	theEncoder.data(theInBuf, theInSize);
	theEncoder.space(theOutBuf, theOutCapacity);
	const Stream::ZFlush flush = doSyncFlush ? Stream::zSyncFlush :
		Stream::zNoFlush;
	if (!theEncoder.perform(spaceDelta, dataDelta, flush)) {
		theEncodeError = true;
		return false;
	}

	// write all data, decoder may need space to make progress
	theStream.write(theOutBuf, spaceDelta);
	if (theStream.fail())
		return false;

	// move leftovers to the beginning of the buffer
	theInSize -= dataDelta;
	memmove(theInBuf, theInBuf + dataDelta, theInSize);

	return true;
}


/* IFStream */

zlib::IFStream::IFStream(istream &stream): FStream<istream, Inflator>(stream),
	theOutSize(0) {
	thePos = theStream.tellg();
	const int capacity = 16 * 1024;
	init(capacity, capacity * 4);
}

bool zlib::IFStream::eof() const {
	return theInSize <= 0 && theOutSize <= 0 && theStream.eof();
}

bool zlib::IFStream::fail() const {
	return theOutSize <= 0 && // no data is available for reading
		(theEncodeError || (theInSize <= 0 && theStream.fail()));
}

int zlib::IFStream::read(char *buf, int len) {
	int readCount = 0;
	while (len > 0) {
		if (theOutSize > 0) {
			const int size = Min(len, theOutSize);
			if (buf) {
				memmove(buf, theOutBuf, size);
				buf += size;
			}
			len -= size;
			readCount += size;
			theOutSize -= size;
			thePos += size;
			memmove(theOutBuf, theOutBuf + size, theOutSize);
		} else if (!readAndInflate())
			break;
	}
	return readCount;
}

bool zlib::IFStream::readAndInflate() {
	Must(!theOutSize);

	if (theEncodeError)
		return false;

	// read more data, decoder may need it to make progress
	if (theInSize < theInCapacity && theStream.good()) {
		theStream.read(theInBuf + theInSize, theInCapacity - theInSize);
		theInSize += theStream.gcount();
	}

	if (theInSize <= 0)
		return false;

	Size spaceDelta = 0;
	Size dataDelta = 0;
	theEncoder.data(theInBuf, theInSize);
	theEncoder.space(theOutBuf, theOutCapacity);
	if (!theEncoder.perform(spaceDelta, dataDelta, Stream::zSyncFlush)) {
		theEncodeError = true;
		return false;
	}

	// move leftovers to the beginning of the buffer
	theInSize -= dataDelta;
	memmove(theInBuf, theInBuf + dataDelta, theInSize);

	theOutSize += spaceDelta;
	if (!theOutSize && theInSize > 0 && theStream.eof()) {
		// leftovers in the input buffer cannot be decompressed
		theEncodeError = true;
		return false;
	}

	return true;
}
