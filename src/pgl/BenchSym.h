
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_BENCHSYM_H
#define POLYGRAPH__PGL_BENCHSYM_H

#include "xstd/Array.h"
#include "xstd/NetAddr.h"
#include "pgl/PglRecSym.h"

class BenchSideSym;

// bench configuration information
class BenchSym: public RecSym {
	public:
		static String TheType;

	public:
		BenchSym();
		BenchSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		const BenchSideSym *clientSide() const;
		const BenchSideSym *serverSide() const;
		const BenchSideSym *proxySide() const;
		const BenchSideSym *side(const String &name) const;

		bool peakReqRate(double &rate) const;

		String clientHostCount(int &count) const; // returns err if any

	protected:
		virtual SynSym *dupe(const String &dType) const;
		const BenchSideSym *getSide(const String &name) const;
};

#endif
