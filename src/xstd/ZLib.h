
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_ZLIB_H
#define POLYGRAPH__XSTD_ZLIB_H

// C++ wrappers around zlib library (ww.zlib.org)

#if HAVE_LIBZ
#	include <zlib.h>
#else
	struct z_stream_s {};
#	define Z_DEFAULT_COMPRESSION (-1)
#endif

#include "xstd/Size.h"

namespace zlib {

extern const bool Supported;

// zlib stream base class
class Stream: public z_stream_s {
	public:
		enum ZFlush {
#if HAVE_LIBZ
			zNoFlush   = Z_NO_FLUSH,
			zFinish    = Z_FINISH,
			zSyncFlush = Z_SYNC_FLUSH
#else
			zNoFlush, zFinish, zSyncFlush
#endif
		};

	public:
		virtual ~Stream() {}

		void space(void *space, Size size);
		void data(const void *data, Size size);
		bool perform(Size &usedSpace, Size &usedData, const ZFlush flush = zNoFlush);

		bool needMoreSpace() const;
		bool needMoreData() const;

		const char *error() const; // last error message, null if none

	protected:
		void init();
		virtual int run(ZFlush flush) = 0;

	protected:
		enum { stInit, stDone, stError } theState;
		bool doNeedMoreSpace;

		// the magic constants below are taken from zlib.h to force
		// gzip header and footer for the deflated stream
		static const int TheWindowBits = 15 + 16;
};

// zlib stream for deflation
class Deflator: public Stream {
	public:
		Deflator(const int lvl = Z_DEFAULT_COMPRESSION) { init(lvl); }
		virtual ~Deflator() { reset(); }

		int bound(const int sourceLen);

	private:
		void init(const int level);
		void reset();
		virtual int run(ZFlush flush);

	private:
		// default memLevel according to zlib manual
		static const int TheMemLevel = 8;
};

// zlib stream for inflation
class Inflator: public Stream {
	public:
		Inflator() { init(); }
		virtual ~Inflator() { reset(); }

	private:
		void init();
		void reset();
		virtual int run(ZFlush flush);
};

} // namespace zlib

#endif
