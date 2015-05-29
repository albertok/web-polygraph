
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_STRINGARRAYBLOCKS_H
#define POLYGRAPH__BASE_STRINGARRAYBLOCKS_H

#include "xstd/String.h"

class Area;

// interface for one "block" of a string array
class StringArrayBlock {
	public:
		class TypeAnchor {};
		typedef TypeAnchor *Type;

	public:
		StringArrayBlock(Type aType): theType(aType) {}
		virtual ~StringArrayBlock() {}

		virtual StringArrayBlock *clone() const = 0;

		Type type() const { return theType; }

		virtual int count() const = 0;
		virtual bool find(const Area &member, int &idx) const = 0;

		virtual String item(int idx) const = 0;

		bool canMerge(const StringArrayBlock &b) const;
		void merge(const StringArrayBlock &b);

	protected:
		virtual bool canMergeSameType(const StringArrayBlock &b) const = 0;
		virtual void mergeSameType(const StringArrayBlock &b) = 0;

	private:
		Type theType;
};

// a single String
class StringArrayPoint: public StringArrayBlock {
	public:
		StringArrayPoint(const String &aPoint);

		virtual StringArrayBlock *clone() const;

		virtual int count() const;
		virtual bool find(const Area &member, int &idx) const;

		virtual String item(int idx) const;

	protected:
		virtual bool canMergeSameType(const StringArrayBlock &b) const;
		virtual void mergeSameType(const StringArrayBlock &b);

	protected:
		String thePoint;

	private:
		static TypeAnchor TheTypeAnchor;
};

// see StringRange for StringArrayRange

#endif
