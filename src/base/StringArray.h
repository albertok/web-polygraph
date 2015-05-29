
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_STRINGARRAY_H
#define POLYGRAPH__BASE_STRINGARRAY_H

#include "xstd/Array.h"
#include "xstd/String.h"

class Area;
class StringArrayBlock;

// String array optimized for very large number of elements
// assuming very large string arrays are formed primarily from smart members
// such as StringRange rather than simple enumeration of items
class StringArray {
	public:
		StringArray();
		StringArray(const StringArray &a);
		~StringArray();

		void reset();

		int count() const;

		bool find(const Area &member, int &idx) const;

		String operator[](int idx) const { return item(idx); }
		String item(int idx) const;

		StringArray &operator =(const StringArray &a);

		void append(const String &s);
		void absorb(StringArrayBlock *b);

	protected:
		void append(const StringArray &r);

	private:
		Array<StringArrayBlock*> theBlocks;
		mutable int theCount; // cached value
};

#endif
