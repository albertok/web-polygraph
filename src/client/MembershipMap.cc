
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "xstd/RegEx.h"
#include "xstd/gadgets.h"
#include "base/RndPermut.h"
#include "base/TwoWayPermutator.h"
#include "runtime/LogComment.h"
#include "client/AclGroup.h"
#include "pgl/MembershipMapSym.h"
#include "client/MembershipMap.h"


class MMapMember {
	public:
		const UserCred *creds;
		int index;
		MembershipMap::Range groupRange;
};

MembershipMap::MembershipMap(): theGroupsPerMember(0), theMap(new TwoWayPermutator) {
}

MembershipMap::~MembershipMap() {
	delete theMap;
}

void MembershipMap::configure(const MembershipMapSym &cfg, int id) {
	cfg.groupSpace(theGroupSpace);
	cfg.memberSpace(theMemberSpace);
	theGroupsPerMember = cfg.groupsPerMember();

	if (!theGroupsPerMember) {
		cerr << cfg.loc() << "must specify number of groups per user for a MembershipMap" << endl;
		exit(-3);
	}

	theMap->configure(theGroupSpace.count(), GlbPermut(), id);
}
		
bool MembershipMap::hasMember(const UserCred &userCredentials) const {
	int memberIdx = -1;
	return theMemberSpace.find(userCredentials.image().area(0), memberIdx);
}

bool MembershipMap::match(RegEx *regex, const char *userName, int flags, int *errNo) const {
	if (!theGroupSpace.count())
		return false;

	int memberIdx = -1;
	if (!theMemberSpace.find(Area::Create(userName), memberIdx))
		return false;

	if (regex->exactMatching() && !regex->ignoringCase())
		return exactGroupMatch(regex->pattern(), memberIdx);
	else
		return regexGroupMatch(regex, userName, memberIdx, flags, errNo);
}

bool MembershipMap::exactGroupMatch(const String &groupName, const int memberIdx) const {
	int groupIdx = -1;
	if (!theGroupSpace.find(groupName.area(0), groupIdx))
		return false;

	return calcGroupRange(memberIdx).has(groupIdx);
}

bool MembershipMap::regexGroupMatch(RegEx *regex, const char *userName, int memberIdx, int flags, int *errNo) const {
	static bool warned = false;
	if (!warned) {
		Comment(1) << "one-time warning: using non-trivial regular expression " <<
			"with group scope may be [very] computentially expensive for " <<
			"relatively large number of groups; expression: " << 
			regex->image() << "; user: " << userName << endc;
		warned = true;
	}

	// check all group names within the range
	const Range groupRange = calcGroupRange(memberIdx);
	for (int g = groupRange.beg(); g < groupRange.end(); ++g) {
		const int groupIdx = theMap->directLookup(g % theGroupSpace.count());
		if (regex->match(theGroupSpace[groupIdx].cstr(), flags, errNo))
			return true;
	}

	return false;
}

MembershipMap::GroupIterator MembershipMap::groupIterator(const UserCred &userCredentials) const {
	const Area userName = userCredentials.image().area(0); // XXX: name();

	int memberIdx = -1;
	Range groupRange;
	if (theMemberSpace.find(userName, memberIdx))
		groupRange = calcGroupRange(memberIdx);
	return GroupIterator(this, groupRange);
}

static
int MembershipMap_cmpMemberPtrs(const void *p1, const void *p2) {
	const MMapMember *m1 = (const MMapMember*)p1;
	const MMapMember *m2 = (const MMapMember*)p2;
	return m1->groupRange.beg() - m2->groupRange.beg();
}

int MembershipMap::collectUsedGroupNames(const Array<UserCred*> &users, Array<String*> &groupNames) const {
	const int savedCount = groupNames.count();

	Array<MMapMember> members(users.count());
	for (int i = 0; i < users.count(); ++i) {
		int memberIdx = -1;
		if (theMemberSpace.find(users[i]->image().area(0), memberIdx)) {
			MMapMember member;
			member.creds = users[i];
			member.index = memberIdx;
			member.groupRange = calcGroupRange(memberIdx);
			members.append(member);
		}
	}

	qsort(members.items(), members.count(), sizeof(*members.items()),
		&MembershipMap_cmpMemberPtrs);

	// we can now add unique group names with just one range
	Range coveredR;
	for (int m = 0; m < members.count() && coveredR.size() < theGroupSpace.count(); ++m) {
		const Range &curR = members[m].groupRange;
		if (coveredR) {
			Assert(coveredR.beg() <= curR.beg());
			if (Range newR = curR.exceptHead(coveredR)) {
				const int overlap = (newR.end() % theGroupSpace.count()) - 
					coveredR.beg();
				if (overlap > 0)
					newR.endAt(newR.end() - overlap);
				collectUsedGroupNames(groupNames, newR);
				coveredR.endAt(newR.end());
			}
		} else {
			collectUsedGroupNames(groupNames, curR);
			coveredR = curR;
		}
	}

	return groupNames.count() - savedCount;
}

void MembershipMap::collectUsedGroupNames(Array<String*> &groupNames, const Range &range) const {
	for (int g = range.beg(); g < range.end(); ++g) {
		const int groupIdx = theMap->directLookup(g % theGroupSpace.count());
		groupNames.append(new String(theGroupSpace[groupIdx]));
	}
}

String MembershipMap::groupNameAt(int rangedIdx) const {
	const int groupIdx = theMap->directLookup(rangedIdx % theGroupSpace.count());
	return theGroupSpace[groupIdx];
}

inline static
bool MembershipMap_IsBetween(int beg, int idx, int end) {
	return beg <= idx && idx < end;
}

bool MembershipMap::groupBelongs(int groupIdx, int groupRangeBeg, int groupRangeSize) const {
	const int groupRangedIdx = theMap->reverseLookup(groupIdx);

	const int groupRangeEnd = groupRangeBeg + groupRangeSize;

	// group does not wrap
	if (groupRangeEnd <= theGroupSpace.count())
		return MembershipMap_IsBetween(groupRangeBeg, groupRangedIdx, groupRangeEnd);

	// group wraps
	const int leftovers = groupRangeEnd % theGroupSpace.count();
	return MembershipMap_IsBetween(0, groupRangedIdx, leftovers) ||
		MembershipMap_IsBetween(groupRangeBeg, groupRangedIdx, theGroupSpace.count());
}

MembershipMap::Range MembershipMap::calcGroupRange(int memberIdx) const {
	if (const int maxSize = theGroupSpace.count()) {
		const long beg = GlbPermut(memberIdx, rndMembershipRangeBeg) % maxSize;
		const long size = Max(1L, theGroupsPerMember->ltrial() % maxSize);
		return Range(beg, beg + size);
	} else {
		return Range();
	}
}

