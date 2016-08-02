
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "pgl/PglNetAddrSym.h"
#include "pgl/PglContainerSym.h"
#include "runtime/AddrSubsts.h"


AddrSubsts *TheAddrSubsts = 0;


AddrSubsts::AddrSubsts(): theCount(0) {
}

AddrSubsts::~AddrSubsts() {
	while (theGroups.count()) delete theGroups.pop();
}

void AddrSubsts::configure(const Array<ContainerSym*> &groups) {
	for (int i = 0; i < groups.count(); ++i) {
		ContainerSym &gs = *groups[i];
		if (const int gcount = gs.count()) {
			theGroups.append(new Group(gcount));
			theGroups.last()->configure(gs);
			theCount += gcount;
		}
	}
}

AddrSubsts::Iterator AddrSubsts::iterator(const NetAddr &orig) const {
	return Iterator(*this, orig);
}

const NetAddr &AddrSubsts::at(int group, int idx) const {
	return *theGroups[group]->item(idx);
}


/* AddrSubstGroup */

AddrSubstGroup::AddrSubstGroup(int aCap): Array<NetAddr*>(aCap) {
}

void AddrSubstGroup::configure(const ContainerSym &group) {
	const int cnt = group.count();
	stretch(cnt);
	for (int i = 0; i < cnt; ++i) {
		const NetAddrSym &as = (const NetAddrSym&)group[i]->cast("addr");
		append(new NetAddr(as.val()));
	}
}

bool AddrSubstGroup::member(const NetAddr &addr) const {
	for (int i = 0; i < count(); ++i) {
		if (*item(i) == addr)
			return true;
	}
	return false;
}

/* AddrSubstsIter */

AddrSubstsIter::AddrSubstsIter(const AddrSubsts &aSubs, const NetAddr &anOrig):
	theSubs(aSubs), theOrig(anOrig), theGroupIdx(0), theAddrIdx(0), checkMemb(false) {
	sync();
}

const NetAddr &AddrSubstsIter::addr() {
	return theSubs.at(theGroupIdx, theAddrIdx);
}

bool AddrSubstsIter::atEnd() const {
	return theGroupIdx >= theSubs.theGroups.count();
}

void AddrSubstsIter::sync() {
	while (!atEnd()) {
		const AddrSubstGroup &grp = *theSubs.theGroups[theGroupIdx];
		if (theAddrIdx >= grp.count()) {
			theGroupIdx++;
			theAddrIdx = 0;
			checkMemb = false;
			continue;
		}

		if (!checkMemb) {
			if (!grp.member(theOrig)) {
				theGroupIdx++;
				theAddrIdx = 0;
				checkMemb = false;
				continue;
			}
			checkMemb = true;
		}

		if (*grp[theAddrIdx] == theOrig)
			theAddrIdx++;
		else
			break;
	}
}
