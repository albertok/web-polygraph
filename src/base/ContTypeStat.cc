
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "base/ContTypeStat.h"

static PtrArray<String*> TheKinds;
static int TheNormalContentStart = -1;

void ContType::Record(const int ctype, const String &kind) {
	Assert(ctype == TheKinds.count());
	Assert(kind);
	TheKinds.put(new String(kind), ctype);
}

int ContType::Count() {
	return TheKinds.count();
}

const String &ContType::Kind(const int idx) {
	Must(0 <= idx && idx < Count());
	const String *const kind = TheKinds[idx];
	Must(kind);
	return *kind;
}

void ContType::NoteNormalContentStart() {
	Must(TheNormalContentStart < 0);
	TheNormalContentStart = TheKinds.count();
}

int ContType::NormalContentStart() {
	return TheNormalContentStart;
}

void ContType::Store(OLog &ol) {
	ol << TheKinds << TheNormalContentStart;
}

void ContType::Load(ILog &il) {
	if (TheKinds.empty()) {
		Must(TheNormalContentStart < 0);
		il >> TheKinds >> TheNormalContentStart;
	} else {
		PtrArray<String*> kinds;
		il >> kinds;
		bool same = TheKinds.count() == kinds.count() &&
			TheNormalContentStart == il.geti();
		for (int i = 0; same && i < kinds.count(); ++i)
			same = *kinds[i] == *TheKinds[i];

		if (!same) {
			cerr << "error: mismatching log files (logged content "
				"types differ):" << endl <<
				"content types in the first log:" << endl;
			for (int i = 0; i < TheKinds.count(); ++i)
				cerr << (i ? ", " : "\t") << *TheKinds[i];
			cerr << endl << "content types in the last log:" << endl;
			for (int i = 0; i < kinds.count(); ++i)
				cerr << (i ? ", " : "\t") << *kinds[i];
			cerr << endl;
			exit(-2);
		}
	}
}
