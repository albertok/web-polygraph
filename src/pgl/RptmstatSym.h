
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_RPTMSTATSYM_H
#define POLYGRAPH__PGL_RPTMSTATSYM_H

#include "xstd/Time.h"
#include "pgl/PglRecSym.h"

// run goal specification
class RptmstatSym: public RecSym {
	public:
		static const String TheType;

	public:
		RptmstatSym();
		RptmstatSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool configured() const;

		Time sampleDur() const;
		bool loadDelta(double &loadDelta) const;
		Time rptmMin() const;
		Time rptmMax() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
