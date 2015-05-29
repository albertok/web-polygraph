
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSTRBLOCKS_H
#define POLYGRAPH__PGL_PGLSTRBLOCKS_H


// represents one "block" of a string [range]
class PglStrBlock {
	public:
		enum Type { sbtNone = 0, sbtPoint, sbtRange, sbtEnd };

	public:
		PglStrBlock(Type aType): theType(aType) {}
		virtual ~PglStrBlock() {}

		int type() const { return theType; }

		virtual int count() const = 0;
		virtual PglStrBlock *clone() const = 0;

		int diffCount(const PglStrBlock &b) const;
		void merge(PglStrBlock &b);

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

class PglStrPointBlock: public PglStrBlock {
	public:
		PglStrPointBlock(const char *aStart, const char *aStop);

		virtual int count() const;
		virtual PglStrBlock *clone() const;

		int countDiffs(const PglStrPointBlock &b) const;
		void mergeWith(const PglStrPointBlock &b);

		virtual bool atLast() const;
		virtual int pos() const;

		virtual void start();
		virtual void next();
		virtual void pos(int aPos);

		virtual void print(ostream &os) const;
		virtual void printCur(ostream &os) const;

	protected:
		const char *theStart;
		const char *theStop;
};

class PglStrRangeBlock: public PglStrBlock {
	public:
		PglStrRangeBlock(int aStart, int aStop, bool beIsolated);

		virtual int count() const;
		virtual PglStrBlock *clone() const;

		int countDiffs(const PglStrRangeBlock &b) const;
		void mergeWith(const PglStrRangeBlock &b);

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

		int thePos;
		
		bool isIsolated;
};

#endif
