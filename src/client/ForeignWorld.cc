
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
#include "csm/ContentCfg.h"
#include "pgl/PglSemx.h"


int ForeignWorld::TheWss = 0;

ForeignWorld::ForeignWorld() {
}

void ForeignWorld::configure(const String &aName) {
	Assert(!id());
	id(UniqId::Create());
	type(TheForeignContentId);
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
	setType(oid);
	ObjWorld::repeat(oid, sel);
	setUrl(oid);
}

void ForeignWorld::produce(ObjId &oid, RndGen &rng) {
	setType(oid);
	ObjWorld::produce(oid, rng);
	incWss(TheWss);
	setUrl(oid);
}

void ForeignWorld::setType(ObjId &oid) const {
	Assert(oid.type() < 0);
	oid.type(theType);
}

void ForeignWorld::setUrl(ObjId &oid) const {
	Assert(oid.name() > 0);

	static const String urlNumMacro("${url_number}");
	static const String workerMacro("${worker}");

	static const size_t size(8);
	static char urlNumBuf[size + 1];
	ofixedstream os(urlNumBuf, size + 1);
	os << hex << setfill('0') << setw(size) << oid.name();
	urlNumBuf[size] = '\0';

	String url(*theUrls[oid.name() % theUrls.count()]);
	url = ExpandMacro(url, urlNumMacro, urlNumBuf);
	url = ExpandMacro(url, workerMacro, PglSemx::WorkerIdStr());
	oid.foreignUrl(url); // now set real URL
}
