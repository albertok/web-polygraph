
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_ZLIB_H
#define POLYGRAPH__XSTD_ZLIB_H

// C++ wrappers around zlib library (ww.zlib.org)

#if HAVE_LIBZ
#	include <zlib.h>
#else
	typedef struct z_stream_s {};
#	define Z_DEFAULT_COMPRESSION (-1)
#endif

#include "xstd/Size.h"

// zlib stream for deflation
class Deflator: public z_stream_s {
	public:
		static const bool Supported;

	public:
		Deflator(int level = Z_DEFAULT_COMPRESSION);
		~Deflator();

		void init(int level);
		void reset();

		void space(void *space, Size size);
		void data(const void *data, Size size);

		bool deflate(Size &usedSpace, Size &usedData, bool finish);

		bool needMoreSpace() const;
		bool needMoreData() const;

		const char *error() const; // last error message, null if none

	private:
		enum { stInit, stDone, stError } theState;
		bool doNeedMoreSpace;
};

#endif
