
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_POPMODELSYM_H
#define POLYGRAPH__PGL_POPMODELSYM_H

#include "pgl/PglRecSym.h"

class String;
class PopDistr;

// server side configuration
class PopModelSym: public RecSym {
	public:
		static String TheType;

	public:
		PopModelSym();
		PopModelSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		PopDistr *popDistr() const;
		bool hotSetFrac(double &ratio) const;
		bool hotSetProb(double &prob) const;
		bool bhrDiscr(double &prob) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
