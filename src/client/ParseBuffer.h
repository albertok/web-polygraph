
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_PARSEBUFFER_H
#define POLYGRAPH__CLIENT_PARSEBUFFER_H

// a read/parse-only buffer with consume() capability
// empty buffer should use the default constructor
class ParseBuffer {
	public:
		ParseBuffer(): theData(""), theSize(0) {}
		ParseBuffer(const char *aData, Size aSize):
			theData(aData), theSize(aSize) { Assert(theData && theSize >= 0); }

		bool empty() const { return theSize <= 0; }
		Size size() const { return theSize; }
		const char *data() const { return theData; }

		ParseBuffer head(Size size) const { return ParseBuffer(data(), size); }

		inline void consume(Size sz);
		void consumeAll() { consume(theSize); }

	protected:
		const char *theData;
		Size theSize;
};

inline
void ParseBuffer::consume(Size sz) {
	if (Should(sz <= theSize)) {
		theData += sz;
		theSize -= sz;
	} else {
		consumeAll();
	}
}

#endif
