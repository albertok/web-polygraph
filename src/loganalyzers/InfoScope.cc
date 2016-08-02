
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "loganalyzers/InfoScopeDim.h"
#include "loganalyzers/InfoScope.h"


InfoScope::InfoScope() {
	theSides = new InfoScopeDim("sides");
	thePhases = new InfoScopeDim("phases");
}

InfoScope::InfoScope(const InfoScope &s) {
	theSides = new InfoScopeDim("sides");
	thePhases = new InfoScopeDim("phases");
	copy(s);
}

InfoScope::~InfoScope() {
	reset();
	delete theSides;
	delete thePhases;
}

bool InfoScope::operator ==(const InfoScope &s) const {
	return image() == s.image();
}

InfoScope &InfoScope::operator =(const InfoScope &s) {
	reset();
	copy(s);
	return *this;
}

InfoScope::operator void*() const {
	return theSides->count() && thePhases->count() ? (void*)-1 : 0;
}

void InfoScope::name(const String &aName) {
	Assert(!theName); // paranoid
	theName = aName;
}

void InfoScope::rename(const String &aName) {
	theName = String();
	name(aName);
}

String InfoScope::name() const {
	return theName ? theName : image();
}

String InfoScope::image() const {
	String buf;
	buf += theSides->image();
	buf += "__";
	buf += thePhases->image();
	return buf;
}

const Array<String*> &InfoScope::sides() const {
	return theSides->names();
}

const Array<String*> &InfoScope::phases() const {
	return thePhases->names();
}

InfoScope InfoScope::oneSide(const String &name) const {
	//Assert(hasSide(name));
	InfoScope s;
	s.theSides->add(name);
	s.thePhases->copy(*thePhases);
	return s;
}

InfoScope InfoScope::onePhase(const String &name) const {
	//Assert(hasPhase(name));
	InfoScope s;
	s.theSides->copy(*theSides);
	s.thePhases->add(name);
	return s;
}

void InfoScope::addSide(const String &name) {
	// we need to keep sides ordered because hasScope() and such use image()
	// equality tests while some code adds sides in random order
	if (name == "client")
		theSides->addFront(name);
	else
		theSides->add(name);
}

void InfoScope::addPhase(const String &name) {
	thePhases->add(name);
}

void InfoScope::add(const InfoScope &scope) {
	for (int s = 0; s < scope.sides().count(); ++s) {
		if (!hasSide(*scope.sides().item(s)))
			addSide(*scope.sides().item(s));
	}
	for (int p = 0; p < scope.phases().count(); ++p) {
		if (!hasPhase(*scope.phases().item(p)))
			addPhase(*scope.phases().item(p));
	}
}

bool InfoScope::hasSide(const String &name) const {
	return theSides->has(name);
}

bool InfoScope::hasPhase(const String &name) const {
	return thePhases->has(name);
}

void InfoScope::copy(const InfoScope &s) {
	if (s.theSides != theSides && s.thePhases != thePhases) {
		Assert(!*this);
		reason = s.reason;
		theName = s.theName;
		theSides->copy(*s.theSides);
		thePhases->copy(*s.thePhases);
	}
}

void InfoScope::reset() {
	reason = String();
	theName = String();
	theSides->reset();
	thePhases->reset();
}
