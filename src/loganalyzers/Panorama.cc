
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/AnyToString.h"
#include "loganalyzers/Panorama.h"
#include "loganalyzers/Formatter.h"


String Panorama::TheId = "Panorama";
String PanAtom::TheId = "PanAtom";

Panorama::LocationLabels Panorama::TheLocationLabels;

void Panorama::LabelLocation(const String &location, const String &label) {
	String res;
	if (!TheLocationLabels.find(location, res)) {
		TheLocationLabels.add(location, label);
	} else {
		cerr << "warning: found several test results with identical location:"
			<< endl << "\t" << location << endl;
	}
}

String Panorama::LocationLabel(const String &location) {
	String res = "unlabeled";
	(void)TheLocationLabels.find(location, res);
	return res;
}

Panorama *Panorama::findSlice(const String &skey, int idxHint) {
	Assert(idxHint >= 0);

	// try hinted location first
	if (0 <= idxHint && idxHint < theKids.count()) {
		Panorama *kid = panKid(idxHint);
		if (kid->key() == skey)
			return kid;
	}

	// search for the key
	for (int i = 0; i < theKids.count(); ++i) {
		Panorama *kid = panKid(i);
		if (kid->key() == skey)
			return kid;
	}

	// not found; caller may add a slice
	return 0;
}

Panorama *Panorama::panKid(int idx) {
	Sample *s = theKids[idx];
	Assert(s->typeId() == Panorama::TheId || s->typeId() == PanAtom::TheId);
	return (Panorama*)s;
}

const Panorama *Panorama::panKid(int idx) const {
	Assert(0 <= idx && idx <= theKids.count());
	const Sample *s = theKids[idx];
	Assert(s->typeId() == Panorama::TheId || s->typeId() == PanAtom::TheId);
	return (const Panorama*)s;
}

Panorama *Panorama::genDiff() const {
	Panorama *diff = 0;
	for (int i = 0; i < theKids.count(); ++i) {
		if (Panorama *kidDiff = panKid(i)->genDiff()) {
			if (!diff) {
				diff = new Panorama();
				diff->key(key());
				diff->title(title());
				diff->location(location());
			}
			diff->add(kidDiff);
		}
	}
	return diff;
}

void Panorama::report(Formatter &form) const {
	form.openSection(key(), title());
	for (int i = 0; i < theKids.count(); ++i)
		panKid(i)->report(form);
	form.closeSection();
}


Panorama *PanAtom::genDiff() const {
	bool same = true;
	for (int k = 0; same && k < theKids.count(); ++k) {
		Sample &kid = *theKids[k];
		for (int t = k+1; same && t < theKids.count(); ++t)
			same = theKids[t]->similar(kid);
	}

	if (same)
		return 0;
	
	PanAtom *diff = new PanAtom;
	diff->key(key());
	diff->title(title());
	diff->location(location());
	for (int i = 0; i < theKids.count(); ++i)
		diff->add(theKids[i]->clone());
	return diff;
}

void PanAtom::report(Formatter &form) const {
	if (theKids.count() == 0)
		return;

	form.openSection(key(), title());

	form.openTableAnonym();
	for (int i = 0; i < theKids.count(); ++i) {
		const Sample &kid = *theKids[i];
		form.openTableRecord();

		form.openTableCell("rep-link");
		form.addLink(kid.location(), LocationLabel(kid.location()));
		form.addText(":");
		form.closeTableCell();

		form.openTableCell(kid.typeId());
		form.addLink(kid.location() + '#' + kid.key(), PrintToString(kid));
		form.closeTableCell();

		form.openTableCell("diff");
		const Sample &otherKid = i > 0 ? *theKids[i-1] : *theKids.last();
		kid.reportDifferences(otherKid, form);
		form.closeTableCell();

		form.closeTableRecord();
	}
	form.closeTable();

	form.closeSection();
}
