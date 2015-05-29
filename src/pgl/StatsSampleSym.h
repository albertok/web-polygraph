
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_STATSSAMPLESYM_H
#define POLYGRAPH__PGL_STATSSAMPLESYM_H

#include "xstd/String.h"
#include "pgl/PglRecSym.h"

class StatsSampleCfg;

// describes properties of a single stats sample
class StatsSampleSym: public RecSym {
	public:
		static const String TheType;

	public:
		StatsSampleSym();
		StatsSampleSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		String name() const;
		Time start() const;
		bool capacity(int &cap) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
