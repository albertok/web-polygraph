
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/Assert.h"
#include "xstd/ZLib.h"

// XXX: we should set the [ZLIB] error when ZLIB library is not found
// XXX: we should set the global error to an ZLIB error when an ZLIB call fails

#ifdef HAVE_LIBZ
	const bool Deflator::Supported = true;
#else
	const bool Deflator::Supported = false;
#endif

Deflator::Deflator(int level) {
	init(level);
}

Deflator::~Deflator() {
	reset();
}

void Deflator::init(int level) {
#ifdef HAVE_LIBZ
	doNeedMoreSpace = false;
	zalloc = Z_NULL;
	zfree = Z_NULL;
	opaque = 0;
	// the magic constants below are taken from zlib.h to force
	// gzip header and footer for the deflated stream
	int res = deflateInit2(this, level, Z_DEFLATED, 15 + 16, 8,
		Z_DEFAULT_STRATEGY);
	theState = Should(res == Z_OK) ? stInit : stError;
#else
	theState = stError;
#endif
}

void Deflator::reset() {
#ifdef HAVE_LIBZ
	 deflateEnd(this);
#endif
}

void Deflator::space(void *space, Size size) {
#ifdef HAVE_LIBZ
	next_out = (Bytef*)space;
	avail_out = size;
#endif
}

void Deflator::data(const void *data, Size size) {
#ifdef HAVE_LIBZ
	next_in = (Bytef*)data;
	avail_in = size;
#endif
}

bool Deflator::deflate(Size &usedSpace, Size &usedData, bool finish) {
#ifdef HAVE_LIBZ
	Should(avail_out > 0);
	if (!Should(theState == stInit))
		return false;

	//const Bytef *saved_next_in = next_in;
	const uInt saved_avail_in = avail_in;
	//const Bytef *saved_next_out = next_out;
	const uInt saved_avail_out = avail_out;

	const int res = ::deflate(this, finish ? Z_FINISH : Z_NO_FLUSH);
	doNeedMoreSpace = (finish && res == Z_OK) || (res == Z_BUF_ERROR);

	if (res == Z_STREAM_END)
		theState = stDone;

	if (res == Z_OK || res == Z_STREAM_END) {
		usedSpace = saved_avail_out - avail_out;
		usedData = saved_avail_in - avail_in;
		return true;
	}

	if (res == Z_BUF_ERROR)
		return true; // no progress was possible, but may be not an error

	return false;
#else
	return false;
#endif
}

bool Deflator::needMoreSpace() const {
#ifdef HAVE_LIBZ
	return theState < stDone && doNeedMoreSpace;
#else
	return false;
#endif
}

bool Deflator::needMoreData() const {
#ifdef HAVE_LIBZ
	return avail_in <= 0 && theState < stDone;
#else
	return false;
#endif
}

const char *Deflator::error() const {
#ifdef HAVE_LIBZ
    return msg;
#else
	return "support for zlib is disabled at configuration time";
#endif
}
