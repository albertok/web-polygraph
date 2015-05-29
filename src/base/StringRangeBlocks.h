
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_STRINGRANGEBLOCKS_H
#define POLYGRAPH__BASE_STRINGRANGEBLOCKS_H

#include "xstd/String.h"

// interface for one "block" of a string range
class StringRangeBlock {
	public:
		enum Type { sbtNone = 0, sbtPoint, sbtInterval, sbtEnd };

	public:
		StringRangeBlock(Type aType): theType(aType) {}
		virtual ~StringRangeBlock() {}

		int type() const { return theType; }

		virtual int count() const = 0;
		virtual StringRangeBlock *clone() const = 0;

		virtual bool findTail(const Area &a, int &tailPos, int &idx) const = 0;

		int diffCount(const StringRangeBlock &b) const;
		void merge(StringRangeBlock &b);

		virtual bool atLast() const = 0;
		virtual int pos() const = 0;

		virtual void start() = 0;
		virtual void next() = 0;
		virtual void pos(int aPos) = 0;

		virtual void print(ostream &os) const = 0;
		virtual void printCur(ostream &os) const = 0;

	protected:
		int theType;
};

// single point within a range
class StringRangePoint: public StringRangeBlock {
	public:
		StringRangePoint(const String &aPoint);

		virtual int count() const;
		virtual StringRangeBlock *clone() const;

		virtual bool findTail(const Area &a, int &tailPos, int &idx) const;

		int countDiffs(const StringRangePoint &b) const;
		void mergeWith(const StringRangePoint &b);

		virtual bool atLast() const;
		virtual int pos() const;

		virtual void start();
		virtual void next();
		virtual void pos(int aPos);

		virtual void print(ostream &os) const;
		virtual void printCur(ostream &os) const;

	protected:
		String thePoint;
};

// an interval within a range
class StringRangeInterval: public StringRangeBlock {
	public:
		StringRangeInterval(int aStart, int aStop, bool beIsolated, int aBase);

		virtual int count() const;
		virtual StringRangeBlock *clone() const;

		virtual bool findTail(const Area &a, int &tailPos, int &idx) const;

		int countDiffs(const StringRangeInterval &b) const;
		void mergeWith(const StringRangeInterval &b);

		virtual bool atLast() const;
		virtual int pos() const;

		virtual void start();
		virtual void next();
		virtual void pos(int aPos);

		virtual void print(ostream &os) const;
		virtual void printCur(ostream &os) const;

	protected:
		bool contains(int pos) const;

	protected:
		int theStart;
		int theStop;
		int theBase;

		int thePos;
		
		bool isIsolated;
};

#endif
