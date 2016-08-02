
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_MEMBERSHIPMAP_H
#define POLYGRAPH__CLIENT_MEMBERSHIPMAP_H

#include "xstd/Interval.h"
#include "base/StringArray.h"
#include "runtime/UserCred.h"
#include "client/MembershipGroupIterator.h"

class RegEx;
class RndDistr;
class TwoWayPermutator;
class MembershipMapSym;

class MembershipMap {
	public:
		friend class MembershipGroupIterator;
		typedef MembershipGroupIterator GroupIterator;
		typedef Interval Range;

	public:
		MembershipMap();
		~MembershipMap();

		void configure(const MembershipMapSym &cfg, int id);

		int groupNameCount() const { return theGroupSpace.count(); }
		int userNameCount() const { return theMemberSpace.count(); }
		
		bool hasMember(const UserCred &userCredentials) const;
		bool match(RegEx *regex, const char *userName, int flags, int *errNo) const;

		GroupIterator groupIterator(const UserCred &userCredentials) const;
		int collectUsedGroupNames(const Array<UserCred*> &users, Array<String*> &groupNames) const;

	protected:
		String groupNameAt(int rangedIdx) const;
		void collectUsedGroupNames(Array<String*> &groupNames, const Range &range) const;

		bool exactGroupMatch(const String &groupName, const int memberIdx) const;
		bool regexGroupMatch(RegEx *regex, const char *userName, int memberIdx, int flags, int *errNo) const;
		bool groupBelongs(int groupIdx, int groupRangeBeg, int groupRangeSize) const;
		Range calcGroupRange(int memberIdx) const;

	protected:
		StringArray theGroupSpace;
		StringArray theMemberSpace;
		RndDistr *theGroupsPerMember;

		TwoWayPermutator *theMap;
};

class Memberships: public Array<MembershipMap*> {
	public:
		Memberships(int aCapacity = 0): Array<MembershipMap*>(aCapacity) {}
};

#endif
