
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_AREA_H
#define POLYGRAPH__XSTD_AREA_H

#include "xstd/h/iosfwd.h"


// a range of bytes from a buffer that is assumed to outlive Area
// meant for referencing an area in an existing buffer
// not meant for storage of bytes
class Area {
	public:
		// to prevent users from using Area for storage
		static Area Create(const char *buf, int offset, int size) {
			return Area(buf, offset, size);
		}
		static Area Create(const char *str);
		static Area Empty() { return Create(0); }

	public:
		operator void *() const { return size() ? (void*)-1 : (void*)0; }
		bool operator !() const { return size() == 0; }

		int size() const { return theSize; }

		// this is not a cstr(); it may not be 0-terminated
		const char *data() const { return theBuf + theOffset; }

		Area head(int size) const { return Create(theBuf, theOffset, size); }

		char operator [](int idx) const { return theBuf[theOffset+idx]; }

	private:
		Area(const char *aBuf, int anOffset, int aSize):
			theBuf(aBuf), theOffset(anOffset), theSize(aSize) {}

		Area &operator =(const Area &area); // disabled

	protected:
		const char *theBuf;
		int theOffset;
		int theSize;
};

extern ostream &operator <<(ostream &os, const Area &area);


#endif
