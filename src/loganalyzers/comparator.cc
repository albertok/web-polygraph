
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include "xstd/h/iostream.h"
#include <fstream>
#include "xstd/h/iomanip.h"
#include "xstd/gadgets.h"
#include "base/AnyToString.h"

#include <functional>
#include <list>

#include "base/CmdLine.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/CompOpts.h"
#include "loganalyzers/Sample.h"
#include "loganalyzers/Panorama.h"
#include "loganalyzers/Formatter.h"

#include "Hapy/Rule.h"
#include "Hapy/Rules.h"
#include "Hapy/Parser.h"


typedef list<Sample*> Samples;

Hapy::Rule rGrammar("grammar", 0);
Hapy::Rule rNode("node", 0);
Hapy::Rule rPi("pi", 0);
Hapy::Rule rTag("tag", 0);
Hapy::Rule rOpenTag("open-tag", 0);
Hapy::Rule rCloseTag("close-tag", 0);
Hapy::Rule rClosedElement("closed-element", 0);
Hapy::Rule rText("text", 0);
Hapy::Rule rAttr("attr", 0);
Hapy::Rule rName("name", 0);
Hapy::Rule rValue("value", 0);

inline
bool operator ==(const String &s1, const std::string &s2) {
	return s1 == String(s2.c_str());
}
inline
bool operator ==(const std::string &s2, const String &s1) {
	return s1 == String(s2.c_str());
}


static
void buildGrammar() {
	using namespace Hapy;

	rGrammar = *rNode;
	rNode = rPi | rTag | rText;
	rTag = rOpenTag | rCloseTag | rClosedElement;

	rPi = "<?" >> rName >> *(anychar_r - "?>") >> "?>";

	rOpenTag = "<" >> rName >> *rAttr >> ">";
	rCloseTag = "</" >> rName >> ">";
	rClosedElement = "<" >> rName >> *rAttr >> "/>";

	rText = +(anychar_r - '<');

	rAttr = rName >> '=' >> rValue;
	rName = alpha_r >> *(alnum_r | '_' | ':');
	rValue = quoted_r(anychar_r, '"') | quoted_r(anychar_r, "'");

	// trimming rules
	rGrammar.trim(*space_r);
	rText.verbatim(true);
	rName.verbatim(true);
	rValue.verbatim(true);

	// parse tree shaping rules
	rText.leaf(true);
	rName.leaf(true);
	rValue.leaf(true);

	// parsing optimization rules
	rGrammar.committed(true);
	rText.committed(true);
	rName.committed(true);
	rValue.committed(true);
	rNode.committed(true);
}

static
void parseFile(ifstream &is, Hapy::Parser &parser) {
	is.unsetf(std::ios::skipws);

	string content;
	char c;
	while (is.get(c))
		content += c;

	parser.grammar(rGrammar);
	if (!parser.parse(content)) {
		cerr << parser.result().location() << ": syntax error" << endl;
		exit(2);
	}
}

static
bool findAttr(const Hapy::Pree &tag, const String &name, String *value) {
	// "<" >> rName >> *rAttr >> ">";
	const Hapy::Pree &attrs = tag[2];
	for (Hapy::Pree::const_iterator i = attrs.begin(); i < attrs.end(); ++i) {
		// rAttr = rName >> '=' >> rValue;
		const Hapy::Pree &attr = *i;
		if (attr[0].image() == name) {
			if (value) {
				const Hapy::Pree &v = attr[2];
				if (Should(v.image().size() >= 2))
					*value = String(v.image().substr(
						1,v.image().size()-2).c_str());
			}
			return true;
		}
	}
	return false;
}


typedef Hapy::Pree::const_iterator PreeIter;
static Sample *skipSample(PreeIter &begin, const PreeIter &end);

static
bool findCompositeSampleEnd(const PreeIter &begin, PreeIter &end, const Hapy::Pree &opener, CompositeSample *c) {
	const Hapy::string tagName = opener[1].image();
	for (PreeIter i = begin; i != end;) {
		const Hapy::Pree &node = *i;

		if (node.rid() == rNode.id()) {
			// rNode = rPi | rTag | rText;
			const Hapy::Pree &pree = node[0];
				
			if (pree.rid() == rTag.id()) {
				// rTag = rOpenTag | rCloseTag | rClosedElement;
				const Hapy::Pree &p = pree[0];
				if (p.rid() == rCloseTag.id() && p[1].image() == tagName) {
					end = i+1;
					return true;
				}
			}
		}

		if (Sample *kid = skipSample(i, end))
			c->add(kid);
		else
			++i;
	}

	cerr << "warning: skipping open sample: " << c->key() << endl;
	return false;
}

static
bool findAtomSampleEnd(const PreeIter &begin, PreeIter &end, const Hapy::Pree &opener, String &image) {
	const Hapy::string tagName = opener[1].image();
	for (PreeIter i = begin; i != end; ++i) {
		const Hapy::Pree &node = *i;

		if (node.rid() == rNode.id()) {
			// rNode = rPi | rTag | rText;
			const Hapy::Pree &pree = node[0];

			if (pree.rid() == rText.id()) {
				image += String(pree.image().c_str());
				continue;
			}

			if (pree.rid() == rTag.id()) {
				// rTag = rOpenTag | rCloseTag | rClosedElement;
				const Hapy::Pree &p = pree[0];
				if (p.rid() == rCloseTag.id() && p[1].image() == tagName) {
					end = i+1;
					return true;
				}
			}

			if (pree.rid() == rPi.id())
				continue;
		}
		cerr << "warning: ignoring non-text component of a text sample " <<
			"near " << node.image() << endl;
	}
	cerr << "warning: skipping open sample near " << begin->image() << endl;
	return false;
}

static
Sample *skipSample(PreeIter &begin, const PreeIter &end) {
	const Hapy::Pree &node = *begin;

	if (node.rid() != rNode.id())
		return 0;
	const Hapy::Pree &pree = node[0];

	if (pree.rid() != rTag.id())
		return 0;

	// rTag = rOpenTag | rCloseTag | rClosedElement;
	const Hapy::Pree &tag = pree[0];
	const bool kids = tag.rid() == rOpenTag.id();

	// is it a Sample element?
	String attrId;
	String attrClass;
	String attrTitle;
	if (!findAttr(tag, "id", &attrId) ||
		!findAttr(tag, "class", &attrClass) ||
		!findAttr(tag, "title", &attrTitle))
		return 0;

	// possibly a Sample element, check class
	Sample *s = 0;
	if (attrClass == CompositeSample::TheId) {
		CompositeSample *c = new CompositeSample;
		if (kids) {
			PreeIter b = begin + 1;
			PreeIter e = end;
			if (findCompositeSampleEnd(b, e, tag, c)) {
				begin = e;
			} else {
				delete c;
				c = 0;
			}
		}
		s = c;
	} else
	if (attrClass == NumberSample::TheId) {
		NumberSample *n = new NumberSample;
		if (Should(kids)) {
			PreeIter b = begin + 1;
			PreeIter e = end;
			String buf;
			if (findAtomSampleEnd(b, e, tag, buf)) {
				begin = e;
				n->setImage(buf);
			} else {
				delete n;
				n = 0;
			}
		}
		s = n;
	} else
	if (attrClass == TextSample::TheId) {
		TextSample *t = new TextSample;
		if (Should(kids)) {
			PreeIter b = begin + 1;
			PreeIter e = end;
			String buf;
			if (findAtomSampleEnd(b, e, tag, buf)) {
				begin = e;
				t->setImage(buf);
			} else {
				delete t;
				t = 0;
			}
		}
		s = t;
	}

	if (s) {
		s->key(attrId);
		s->title(attrTitle);
	}

	return s;
}

static
void scanAll(Samples &samples) {
	buildGrammar();

	for (int i = 0; i < TheCompOpts.theReports.count(); ++i) {
		const String &fname = *TheCompOpts.theReports[i];
		clog << "scanning " << fname << endl;

		Hapy::Parser parser;
		ifstream f(fname.cstr(), ios::in);
		parseFile(f, parser);
		const Hapy::Pree &pree = parser.result().pree;

		CompositeSample *s = new CompositeSample;
		s->key("");
		s->title("TBD");

		for (PreeIter p = pree.begin(); p < pree.end();) {
			if (Sample *kid = skipSample(p, pree.end()))
				s->add(kid);
			else
				++p;
		}

		s->propagateLocation(fname);
		s->title(Panorama::LocationLabel(fname));

		if (s->kidCount()) {
			samples.push_back(s);
		} else {
			clog << "warning: no samples detected in " << fname <<
				", skipping" << endl;
			delete s;
		}
	}
}

static
void buildReport(const Samples &samples) {
	Assert(samples.size() >= 2); // XXX: enforce in options
	const Sample *cur = *samples.begin();

	/*clog << here << "built sample tree" << endl;
	for (Samples::const_iterator i = samples.begin(); i != samples.end(); ++i)
		(*i)->print(clog);*/

	// make panorama
	Panorama *pan = cur->makePanoramaSkeleton();
	for (Samples::const_iterator i = samples.begin(); i != samples.end(); ++i)
		(*i)->fillPanorama(pan);

	/*clog << here << "built panorama" << endl;
	pan->print(clog);*/

	Panorama *diff = pan->genDiff();

	WebPageFormatter formatter(&cout);
	//formatter.openPage();

	if (TheCompOpts.theDelta < 0) {
		formatter.addText("Side-by-side comparion, all values are reported.");
	} else {
		formatter.addText(
			"Side-by-side comparion with values different by at least " +
			AnyToString(TheCompOpts.theDelta*100.) + "%.");
	}

	if (diff)
		diff->report(formatter);
	else
		formatter.addText("No values found or no differences detected.");

	//formatter.closePage();
	formatter.make();
}

static
void configure() {
	configureStream(cout, 3);

	double &delta = NumberSample::TheDelta;
	delta = TheCompOpts.theDelta;
	if (delta < 0) {
		clog << "fyi: negative delta specified; " <<
			"even identical values will be reported" << endl;
	} else
	if (delta < 1e-10) {
		clog << "fyi: zero delta specified or implied; " <<
			"all different values will be reported" << endl;
	} else {
		clog << "fyi: " << delta << " delta specified; " <<
			"differences of more than " << 100*delta <<
			"% will be reported" << endl;
	}
}

int main(int argc, char *argv[]) {

	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheCompOpts);
	if (!cmd.parse(argc, argv) || !TheCompOpts.validate())
		return -1;

	configure();
	Samples samples;
	scanAll(samples);

	if (samples.size() >= 2) {
		clog << "comparing..." << endl;
		buildReport(samples);
		return 0;
	} else {
		cerr << "error: no stat samples detected in input file" << endl;
		return 255;
	}
}
