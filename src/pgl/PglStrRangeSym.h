
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSTRRANGESYM_H
#define POLYGRAPH__PGL_PGLSTRRANGESYM_H

#include "pgl/PglStringArrayBlockSym.h"

class PglStrRange;

// a compact representation of a range of strings
class StrRangeSym: public StringArrayBlockSym {
	public:
		static const String TheType;

	public:
		StrRangeSym();
		virtual ~StrRangeSym();

		virtual bool isA(const String &type) const;

		virtual int count() const;
		virtual bool probsSet() const;
		virtual const SynSym *itemProb(int offset, double &prob) const;

		void range(PglStrRange *aRange) { theRange = aRange; }

		virtual const StringArrayBlock &val() const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &type) const;

	protected:
		PglStrRange *theRange;
		mutable StringSym *theStr; // cache for itemProb()
};

#endif
