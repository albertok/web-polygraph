
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/MembershipMap.h"
#include "client/MembershipGroupIterator.h"


MembershipGroupIterator::MembershipGroupIterator(const MembershipMap *aMembership, Interval &aRange):
	theMembership(aMembership), theRange(aRange), thePos(aRange.beg()) {
	sync();
}

String MembershipGroupIterator::operator *() const {
	if (Should(!atEnd()))
		return theMembership->groupNameAt(thePos);
	else
		return String();
}

MembershipGroupIterator &MembershipGroupIterator::operator ++() {
	if (!atEnd()) {
		step();
		sync();
	}
	return *this;
}
