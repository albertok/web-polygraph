
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"
#include "xstd/h/iostream.h"
#include <ctype.h>

#include "base/macros.h"
#include "base/ObjId.h"
#include "base/ForeignTrace.h"
#include "runtime/LogComment.h"
#include "client/ForeignWorld.h"


int ForeignWorld::TheWss = 0;

ForeignWorld::ForeignWorld() {
}

void ForeignWorld::configure(const String &aName) {
	Assert(!id());
	id(UniqId::Create());
	theName = aName;

	Comment(7) << "loading URL trace '" << theName << "'" << endc;

	ForeignTrace trace;
	trace.configure(theName);
	if (trace.gatherUrls(theUrls) == 0) {
		Comment(1) << "error: corrupted or empty trace '" << theName << "'" <<
			endc << xexit;
	}
}

void ForeignWorld::repeat(ObjId &oid, ObjSelector *sel) {
	static const String tag("foreign");
	oid.foreignUrl(tag); // hack: let underlying levels know the oid is foreign
	ObjWorld::repeat(oid, sel);
	setUrl(oid);
}

void ForeignWorld::produce(ObjId &oid, RndGen &rng) {
	ObjWorld::produce(oid, rng);
	incWss(TheWss);
	setUrl(oid);
}

void ForeignWorld::setUrl(ObjId &oid) const {
	Assert(oid.name() > 0);

	static const String macro("${url_number}");
	static const size_t size(8);
	static char buf[size + 1];

	ofixedstream os(buf, size + 1);
	os << hex << setfill('0') << setw(size) << oid.name();
	buf[size] = '\0';
	const String &url(*theUrls[oid.name() % theUrls.count()]);
	oid.foreignUrl(ExpandMacro(url, macro, buf)); // now set real URL
}
