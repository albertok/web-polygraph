
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglListSym.h"



const String ListSym::TheType = "List";



ListSym::ListSym(): ExpressionSym(TheType) {
}

ListSym::ListSym(const String &aType): ExpressionSym(aType) {
}

ListSym::ListSym(const ListSym &aList): Array<SynSym*>(aList.count()), ExpressionSym(aList.type()) {
	Assert(absorb(aList));
}

ListSym::~ListSym() {
	while (count()) delete pop();
}

bool ListSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *ListSym::dupe(const String &type) const {
	if (!isA(type))
		return 0;

	ListSym *clone = new ListSym(TheType);

	if (!clone->absorb(*this)) {
		delete clone;
		return 0;
	}

	return clone;
}

void ListSym::add(const SynSym &s) {
	if (!cadd(s))
		addFailure(s);
}

void ListSym::addFailure(const SynSym &) {
	Assert(0);
}

bool ListSym::cadd(const SynSym &s) {
	doAdd(s.clone());
	return true;
}

void ListSym::clear() {
	while (count()) delete pop();		
}

void ListSym::doAdd(SynSym *sclone) {
	Assert(sclone);
	append(sclone);
}

bool ListSym::absorb(const ListSym &arr) {
	const int scount = count();

	for (int i = 0; i < arr.count(); ++i) {
		Assert(arr[i]);
		if (!cadd(*arr[i])) {
			// restore previous state
			while (count() > scount) delete pop();
			return false;
		}
	}

	return true;
}

ostream &ListSym::print(ostream &os, const String &pfx) const {
	//os << '[';
	for (int i = 0; i < count(); ++i) {
		if (i) os << ',';
		item(i)->print(os, pfx+"    ");
	}
	return os;
	//return os << ']';
}
