
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_STRINGRANGE_H
#define POLYGRAPH__BASE_STRINGRANGE_H

#include "xstd/h/iosfwd.h"
#include "xstd/Array.h"
#include "base/StringArrayBlocks.h"

class StringRangeBlock;

// an efficient string range maintainer (e.g., "www[1-3].blah[1-100].com") 
class StringRange: public StringArrayBlock {
	public:
		typedef void (*Iter)(const String &);

	public:
		StringRange();
		StringRange(const StringRange &r);
		virtual ~StringRange();

		virtual StringArrayBlock *clone() const;
		virtual void reset();

		void currentBase(int aBase);
		int currentBase() const;

		int count() const;
		virtual bool find(const Area &member, int &idx) const;

		void iterate(Iter iter) const;
		String toStr() const;
		void toStrs(Array<String*> &strs) const;
		void strAt(int idx, String &str) const;
		virtual String item(int idx) const;

		virtual ostream &print(ostream &os) const;

		StringRange &operator =(const StringRange &r);

	protected:
		void startIter() const;
		bool nextIter() const;
		bool nextIter(int level) const;
		void skipIter(int count) const;
		void currentIter(String &str) const;
		int iterPos() const;

		int intervalCount() const; // number of interval blocks

		void append(const StringRange &r);
		void addRangePoint(const String &point);
		void addRangeInterval(int start, int stop, bool isolated);

		virtual bool canMergeSameType(const StringArrayBlock &b) const;
		virtual void mergeSameType(const StringArrayBlock &b);

	protected:
		Array<StringRangeBlock*> theBlocks; // range parts

	private:
		static TypeAnchor TheTypeAnchor;
		int theCurrentBase;
};

#endif
