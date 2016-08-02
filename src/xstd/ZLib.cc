
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/Assert.h"
#include "xstd/ZLib.h"

// XXX: we should set the [ZLIB] error when ZLIB library is not found
// XXX: we should set the global error to an ZLIB error when an ZLIB call fails

#ifdef HAVE_LIBZ
	const bool zlib::Supported = true;
#else
	const bool zlib::Supported = false;
#endif

/* Stream */

void zlib::Stream::space(void *space, Size size) {
#ifdef HAVE_LIBZ
	next_out = reinterpret_cast<Bytef *>(space);
	avail_out = size;
#endif
}

void zlib::Stream::data(const void *data, Size size) {
#ifdef HAVE_LIBZ
	next_in = reinterpret_cast<Bytef *>(const_cast<void *>(data));
	avail_in = size;
#endif
}

void zlib::Stream::init() {
#ifdef HAVE_LIBZ
	doNeedMoreSpace = false;
	zalloc = Z_NULL;
	zfree = Z_NULL;
	opaque = 0;
	theState = stInit;
#else
	theState = stError;
#endif
}

bool zlib::Stream::needMoreSpace() const {
#ifdef HAVE_LIBZ
	return theState < stDone && doNeedMoreSpace;
#else
	return false;
#endif
}

bool zlib::Stream::needMoreData() const {
#ifdef HAVE_LIBZ
	return avail_in <= 0 && theState < stDone;
#else
	return false;
#endif
}

const char *zlib::Stream::error() const {
#ifdef HAVE_LIBZ
	return msg;
#else
	return "support for zlib is disabled at configuration time";
#endif
}

bool zlib::Stream::perform(Size &usedSpace, Size &usedData, const ZFlush flush) {
#ifdef HAVE_LIBZ
	Should(avail_out > 0);
	if (!Should(theState == stInit))
		return false;

	//const Bytef *saved_next_in = next_in;
	const uInt saved_avail_in = avail_in;
	//const Bytef *saved_next_out = next_out;
	const uInt saved_avail_out = avail_out;

	const int res = run(flush);
	doNeedMoreSpace = (flush == zFinish && res == Z_OK) ||
		(res == Z_BUF_ERROR);

	if (res == Z_STREAM_END)
		theState = stDone;

	if (res == Z_OK || res == Z_STREAM_END) {
		usedSpace = saved_avail_out - avail_out;
		usedData = saved_avail_in - avail_in;
		return true;
	}

	if (res == Z_BUF_ERROR)
		return true; // no progress was possible, but may be not an error
#endif

	return false;
}


/* Deflator */

int zlib::Deflator::bound(const int sourceLen) {
#ifdef HAVE_LIBZ
	return deflateBound(this, sourceLen);
#else
	return 0;
#endif
}

void zlib::Deflator::init(const int level) {
	Stream::init();
#ifdef HAVE_LIBZ
	const int res = deflateInit2(this, level, Z_DEFLATED, TheWindowBits,
		TheMemLevel, Z_DEFAULT_STRATEGY);
	if (!Should(res == Z_OK))
		theState = stError;
#endif
}

void zlib::Deflator::reset() {
#ifdef HAVE_LIBZ
	deflateEnd(this);
#endif
}

int zlib::Deflator::run(ZFlush flush) {
#ifdef HAVE_LIBZ
	return ::deflate(this, flush);
#else
	Must(false);
	return 0;
#endif
}


/* Inflator */

void zlib::Inflator::init() {
	Stream::init();
#ifdef HAVE_LIBZ
	const int res = inflateInit2(this, TheWindowBits);
	if (!Should(res == Z_OK))
		theState = stError;
#endif
}

void zlib::Inflator::reset() {
#ifdef HAVE_LIBZ
	inflateEnd(this);
#endif
}

int zlib::Inflator::run(ZFlush flush) {
#ifdef HAVE_LIBZ
	return ::inflate(this, flush);
#else
	Must(false);
	return 0;
#endif
}
