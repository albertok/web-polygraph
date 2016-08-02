
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_INFOSCOPE_H
#define POLYGRAPH__LOGANALYZERS_INFOSCOPE_H

#include "xstd/String.h"
#include "xstd/Array.h"

class InfoScopeDim;


// manages scope rules/info
// scope is defined in terms of phase and side names
class InfoScope {
	public:
		InfoScope();
		InfoScope(const InfoScope &s);
		~InfoScope();

		void name(const String &aName);
		void rename(const String &aName);

		operator void*() const;

		String name() const;  // imprecise, human-readable
		String image() const; // precise, for machine use
		const Array<String*> &sides() const;
		const Array<String*> &phases() const;

		// narrow or set
		InfoScope oneSide(const String &name) const;
		InfoScope onePhase(const String &name) const;

		// expand
		void addSide(const String &name);
		void addPhase(const String &name);
		void add(const InfoScope &scope);

		// test
		bool hasSide(const String &name) const;
		bool hasPhase(const String &name) const;

		bool operator ==(const InfoScope &s) const;
		InfoScope &operator =(const InfoScope &s);

		String reason; // why this scope is the way it is

	protected:
		void reset();
		void copy(const InfoScope &s);

	protected:
		String theName;
		InfoScopeDim *theSides;
		InfoScopeDim *thePhases;
};

#endif
