
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_MEMBERSHIPMAPSYM_H
#define POLYGRAPH__PGL_MEMBERSHIPMAPSYM_H

#include "pgl/PglRecSym.h"

class RndDistr;
class StringArray;

// parameters for object life cycle model
class MembershipMapSym: public RecSym {
	public:
		static const String TheType;

	public:
		MembershipMapSym();
		MembershipMapSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool groupSpace(StringArray &names) const;
		bool memberSpace(StringArray &names) const;
		RndDistr *groupsPerMember() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
