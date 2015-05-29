
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "loganalyzers/Sample.h"
#include "loganalyzers/Panorama.h"
#include "loganalyzers/Formatter.h"


String CompositeSample::TheId = "C";
String NumberSample::TheId = "N";
String TextSample::TheId = "T";
double NumberSample::TheDelta = 0;

void Sample::propagateLocation(const String &aLocation) {
	location(aLocation);
}

bool Sample::similar(const Sample &s) const {
	return typeId() == s.typeId() && key() == s.key() ? selfSimilar(s) : false;
}

void Sample::copy(const Sample &s) {
	key(s.key());
	title(s.title());
	location(s.location());
}

void Sample::reportDifferences(const Sample &, Formatter &form) const {
	form.addNothing();
}


CompositeSample::~CompositeSample() {
	for (int i = 0; i < theKids.count(); ++i)
		delete theKids.pop();
}

void CompositeSample::propagateLocation(const String &aLocation) {
	Sample::propagateLocation(aLocation);
	for (int i = 0; i < theKids.count(); ++i)
		theKids[i]->propagateLocation(aLocation);
}

Sample *CompositeSample::clone() const {
	CompositeSample *c = new CompositeSample;
	c->copy(*this);
	return c;
}

void CompositeSample::copy(const CompositeSample &c) {
	Sample::copy(c);
	theKids.reset();
	for (int i = 0; i < c.theKids.count(); ++i)
		add(c.theKids[i]->clone());
}

void CompositeSample::print(ostream &os) const {
	os << this << " {title: " << title() << endl;
	for (int i = 0; i < theKids.count(); ++i)
		theKids[i]->print(os);
	os << this << " }title: " << title() << endl;
}

void CompositeSample::add(Sample *kid) {
	theKids.append(kid);
}

Panorama *CompositeSample::makePanoramaSkeleton() const {
	Panorama *p = new Panorama();
	p->key(key());
	p->title(title());
	for (int i = 0; i < theKids.count(); ++i) {
		p->add(theKids[i]->makePanoramaSkeleton());
	}
	return p;
}

void CompositeSample::fillPanorama(Panorama *p) const {
	for (int i = 0; i < theKids.count(); ++i) {
		Sample *kid = theKids[i];

		Panorama *slice = p->findSlice(kid->key(), i);
		if (!slice) {
			slice = kid->makePanoramaSkeleton();
			p->add(slice);
		}

		theKids[i]->fillPanorama(slice);
	}
}

bool CompositeSample::selfSimilar(const Sample &s) const {
	const CompositeSample &c = static_cast<const CompositeSample&>(s);
	if (theKids.count() != c.theKids.count())
		return false;
	for (int i = 0; i < theKids.count(); ++i) {
		if (!theKids[i]->similar(*c.theKids[i]))
			return false;
	}
	return true;
}

void AtomSample::print(ostream &os) const {
	os << theImage;
}

void AtomSample::setImage(const String &image) {
	theImage = image;
}

Panorama *AtomSample::makePanoramaSkeleton() const {
	Panorama *p = new PanAtom();
	p->key(key());
	p->title(title());
	return p;
}

bool AtomSample::selfSimilar(const Sample &s) const {
	const AtomSample &a = static_cast<const AtomSample&>(s);
	return theImage == a.theImage; // no fuzziness allowed
}



void TextSample::propagateLocation(const String &aLocation) {
	AtomSample::propagateLocation(aLocation);
	if (key() == "label")
		Panorama::LabelLocation(theLocation, image());
}

void TextSample::fillPanorama(Panorama *p) const {
	p->add(clone());
}

Sample *TextSample::clone() const {
	return new TextSample(*this);
}


NumberSample::NumberSample(): theValue(-1) {
}

void NumberSample::setImage(const String &image) {
	AtomSample::setImage(image);
	if (!Should(isNum(image.cstr(), theValue)))
		cerr << "warning: expeted a number, found: '" << image << "'" << endl;
}

void NumberSample::fillPanorama(Panorama *p) const {
	p->add(clone());
}

bool NumberSample::selfSimilar(const Sample &s) const {
	const NumberSample &n = static_cast<const NumberSample&>(s);
	if (TheDelta < 0)
		return true; // different no matter what

	if (image() == n.image())
		return true; // identical

	if (TheDelta == 0)
		return false; // not identical

	const Value diff = Abs(theValue - n.theValue);
	return diff <= TheDelta*Min(Abs(theValue), Abs(n.theValue));
}

Sample *NumberSample::clone() const {
	return new NumberSample(*this);
}

void NumberSample::reportDifferences(const Sample &s, Formatter &form) const {
	if (s.typeId() != TheId) {
		Sample::reportDifferences(s, form);
		return;
	}

	const NumberSample &n = (const NumberSample&)s;

	const Value ref = Abs(n.theValue);
	if (ref < 1e-6) {
		form.addText("infinity");
		return;
	}
		
	const Value diff = theValue - n.theValue;
	form.addInteger((int)(100.*diff/ref + 0.5), "%", true);
}
