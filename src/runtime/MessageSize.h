
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_MESSAGESIZE_H
#define POLYGRAPH__RUNTIME_MESSAGESIZE_H

#include "xstd/Size.h"

class OLog;
class ILog;

// maintains some of the logic of HTTP request or response sizes such as
// missing or incorrect Content-Length headers
class MessageSize {
	public:
		MessageSize();

		void reset();

		Size expected() const { return theExpSize; }
		Size actual() const { return theActSize; }
		bool gotAll() const { return !expectToGetMore(); }
		bool expectToGetMore() const { return theActSize < theExpSize; }
		bool expectToGetLess() const { return theActSize > theExpSize; }
		Size expectToGet(Size availSize) const;

		bool expectingWhatParsed() const { return isExpectingWhatParsed; }
		void expectingWhatParsed(bool be) { isExpectingWhatParsed = be; }

		bool expectedBody() const { return hasExpectedBody; }
		void expectedBody(bool be) { hasExpectedBody = be; }

		void expect(Size sz) { theExpSize = sz; }
		void expectMore(Size sz) { Assert(theExpSize.known()); theExpSize += sz; }
		void got(Size sz) { theActSize += sz; }

		void header(Size sz) { theHdrSize = sz; }
		Size header() const { return theHdrSize; }
		Size actualBody() const { return theHdrSize.known() && theActSize.known() ? theActSize - theHdrSize : Size(); }

		void store(OLog &ol) const;
		void load(ILog &il);

	protected:
		Size theExpSize;  // expected (e.g., Content-Length header)
		Size theActSize;  // incremented as response is received or sent
		Size theHdrSize;  // actual header size
		bool isExpectingWhatParsed; // body parser will determine ExpSize
		bool hasExpectedBody; // message is expected to have a body
};

inline
OLog &operator <<(OLog &ol, const MessageSize &s) {
	s.store(ol);
	return ol;
}

inline
ILog &operator >>(ILog &il, MessageSize &s) {
	s.load(il);
	return il;
}


#endif
