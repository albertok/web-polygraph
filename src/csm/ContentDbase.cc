
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/gadgets.h"
#include "base/BStream.h"
#include "base/AggrStat.h"
#include "runtime/IOBuf.h"
#include "csm/ContentDbase.h"
#include "csm/cdbEntries.h"


/* ContentDbase */

ContentDbase::ContentDbase() {
}

ContentDbase::~ContentDbase() {
	while (theEntries.count()) delete theEntries.pop();
}

int ContentDbase::count() const {
	return theEntries.count();
}

bool ContentDbase::hasLinkOrPage() const {
	for (int i = 0; i < count(); ++i) {
		const int eType = entry(i)->type();
		if (eType == cdbeLink || eType == cdbePage)
			return true;
	}
	return false;
}

double ContentDbase::entrySizeMean() const {
	const int cnt = count();
	if (!cnt)
		return -1;

	AggrStat stat;
	for (int i = 0; i < cnt; ++i)
		stat.record(theEntries[i]->meanSize());
	return stat.mean();
}

static
IBStream &operator >>(IBStream &is, CdbEntry *&e) {
	const int type = is.geti();
	if (is.good()) {
		e = ContentDbase::CreateEntry(type);
		e->load(is);
	} else {
		e = 0;
	}
	return is;
}

static
OBStream &operator <<(OBStream &os, const CdbEntry *e) {
	Assert(e);
	os << e->type();
	e->store(os);
	return os;
}

void ContentDbase::load(IBStream &is) {
	theName = is.name();
	is >> theEntries;
}

void ContentDbase::store(OBStream &os) const {
	theName = os.name();
	os << theEntries;
}

void ContentDbase::add(CdbEntry *e) {
	theEntries.append(e);
}

ostream &ContentDbase::print(ostream &os) const {
	for (int i = 0; i < count(); ++i)
		theEntries[i]->print(os);
	return os;
}

CdbEntry *ContentDbase::CreateEntry(int type) {
	switch (type) {
		case cdbeBlob:
			return new CdbeBlob;
		case cdbeText:
			return new CdbeText;
		case cdbeLink:
			return new CdbeLink;
		case cdbePage:
			return new CdbePage;
		case cdbeComment:
			return new CdbeComment;
		default:
			Assert(false); // unknown entry type
	}
	return 0;
}
