
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_STEXBASE_H
#define POLYGRAPH__LOGANALYZERS_STEXBASE_H

#include "xstd/String.h"

// an algorithm of extracting a value statistics out of a stats record
template <class Value, class Stats>
class StexBase {
	public:
		StexBase(const String &aKey, const String &aName, const String &aUnit):
			theKey(aKey), theName(aName), theUnit(aUnit) {}

		virtual ~StexBase() {}

		const String &key() const { return theKey; } // precise, for machine use
		const String &name() const { return theName; } // imprecise, human-readable
		const String &unit() const { return theUnit; } // measurement unit

		virtual bool valueKnown(const Stats &rec) const = 0;
		virtual Value value(const Stats &rec) const = 0;

	protected:
		String theKey;
		String theName;
		String theUnit;
};

#endif
