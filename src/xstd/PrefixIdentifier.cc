
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iomanip.h"

#include "xstd/String.h"
#include "xstd/PrefixIdentifier.h"


/* PrefixIdentifier */

void PrefixIdentifier::doAdd(String &str, int id) {
	Assert(!isLocked); // no additions if optimize()-d
	theHead.add(&str, id, -1);
}

int PrefixIdentifier::lookup(const String &str) const {
	return lookup(str.data(), str.len());
}

int PrefixIdentifier::lookup(const char *buf, int len) const {
	return theHead.lookup(buf, len);
}

// collapses single-branch paths
void PrefixIdentifier::optimize() {
	// new entries cannot be added from now on
	// note: we can support run-time additions, but Node::add 
	//       must not assume sequntial lookup positions
	isLocked = true; 
	theHead.optimize();
}

void PrefixIdentifier::report(ostream &os) const {
	theHead.report(os, 0);
}


/* PrefixIdNode */

PrefixIdNode::PrefixIdNode(): theId(0) {
	u.theTab = 0;
	u.theStr = 0;
}

void PrefixIdNode::add(String *str, int id, int myPos) {
	if (!theId) { // virgin node
		Assert(id > 0 && str);
		theId = id;
		u.theStr = str;
	} else
	if (theId > 0) { // conflict, create next level table
		String *s = u.theStr;
		u.theTab = new PrefixIdTable(myPos+1);
		u.theTab->add(s, theId);
		u.theTab->add(str, id);
		theId = -1;
	} else {  // intermediate node, move further
		u.theTab->add(str, id);
	}
}

int PrefixIdNode::lookup(const char *buf, int len) const {
	if (!theId)
		return 0;
	else
	if (theId > 0)
		return u.theStr->casePrefixOf(buf, len) ? theId : 0;
	else
		return u.theTab->lookup(buf, len);
}

void PrefixIdNode::optimize() {
	// shrink outgoing branch link by link until hit a fan-out
	PrefixIdNode n;
	while (theId < 0 && u.theTab->single(n)) {
		delete u.theTab;
		*this = n;
	}
	
	// recurse if needed
	if (theId < 0)
		u.theTab->optimize();
}

void PrefixIdNode::report(ostream &os, int myLevel) const {
	if (!theId)
		os << "(0) <none>" << endl;
	else
	if (theId > 0)
		os << '(' << theId << ") " << *u.theStr << endl;
	else
	if (theId < 0)
		u.theTab->report(os, myLevel+1);
}


/* PrefixIdTable */

PrefixIdTable::PrefixIdTable(int aPos): theLookupPos(aPos) {
	Assert(0 <= theLookupPos && theLookupPos <= 255);
}

void PrefixIdTable::add(String *str, int id) {
	const int idx = lookupIdx(*str);
	if (idx >= theNodes.capacity()) // out of bounds
		theNodes.resize(idx + 1);
	theNodes[idx].add(str, id, theLookupPos);
}

int PrefixIdTable::lookup(const char *buf, int len) const {
	const int idx = lookupIdx(buf, len);
	if (idx < 0) // buffer is too short already
		return 0;
	else
	if (idx >= theNodes.count()) // lookup char is out of bounds
		return 0;
	else
		return theNodes[idx].lookup(buf, len);
}

void PrefixIdTable::optimize() {
	for (int i = 0; i < theNodes.count(); ++i) {
		if (!theNodes[i].dead())
			theNodes[i].optimize();
	}
}

bool PrefixIdTable::single(PrefixIdNode &n) {
	// to optimize we could've keep track on the number of live nodes
	// but it is combersome because of node conflicts/propogation
	int idx = -1;
	for (int i = 0; i < theNodes.count(); ++i) {
		if (!theNodes[i].dead()) {
			if (idx >= 0)
				return false; // more than one found
			idx = i;
		}
	}
	if (idx < 0) // none found
		return false;
	n = theNodes[idx];
	return true;
}

void PrefixIdTable::report(ostream &os, int myLevel) const {
	os << endl;
	for (int i = 0; i < theNodes.count(); ++i) {
		if (!theNodes[i].dead()) {
			os << setw(2*myLevel) << " ";
			if (i > 32)
				os << (char)i;
			else
				os << '<' << i << '>';
			os << ": ";
			theNodes[i].report(os, myLevel);
		}
	}
}
