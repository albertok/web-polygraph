
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2012 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_ZFSTREAM_H
#define POLYGRAPH__XSTD_ZFSTREAM_H

#include "xstd/h/iostream.h"

#include "xstd/Size.h"
#include "xstd/ZLib.h"

namespace zlib {

template <class Stream, class Encoder>
class FStream {
	public:
		FStream(Stream &aStream);
		~FStream() { delete theInBuf; delete theOutBuf; }

		typename Stream::pos_type pos() const { return thePos; }
		const char *error() const { return theEncoder.error(); }

	protected:
		void init(const int anInCapacity, const int anOutCapacity);

	protected:
		Stream &theStream; // file stream
		Encoder theEncoder; // zlib encoder
		int theInCapacity; // in buffer size
		char *theInBuf; // in buffer
		int theOutCapacity; // out buffer size
		char *theOutBuf; // out buffer
		int theInSize; // in buffer space used
		typename Stream::pos_type thePos; // position in uncompressed stream
		bool theEncodeError; // encoder error flag
};

class OFStream: public FStream<ostream, Deflator> {
	public:
		OFStream(ostream &stream);

		bool fail() const { return theEncodeError || theStream.fail(); }
		bool write(const char *buf, int len);
		const char *error() const { return theEncoder.error(); }

	private:
		bool deflateAndWrite(const bool doSyncFlush);
};

class IFStream: public FStream<istream, Inflator> {
	public:
		IFStream(istream &aStream);

		bool eof() const;
		bool fail() const;
		int read(char *buf, int len);
		int ignore(const int n) { return read(NULL, n); }

	private:
		bool readAndInflate();

	private:
		int theOutSize; // out buffer space used
};

} // namespace zlib

#endif
