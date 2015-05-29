
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_LOADSTEX_H
#define POLYGRAPH__LOGANALYZERS_LOADSTEX_H

#include "xstd/String.h"
#include "base/StatIntvlRec.h"

// an interface of extracting a particular load statistics out of
// interval stats record
class LoadStex {
	public:
		LoadStex(const String &aKey, const String &aName);
		virtual ~LoadStex() {}

		const String &key() const { return theKey; } // precise, for machine use
		const String &name() const { return theName; } // imprecise, human-readable

		virtual double rate(const StatIntvlRec &rec) const = 0;
		virtual double bwidth(const StatIntvlRec &rec) const = 0;

	protected:
		double perDuration(double meas, const StatIntvlRec &rec) const;

	protected:
		String theKey;
		String theName;
};

#endif
