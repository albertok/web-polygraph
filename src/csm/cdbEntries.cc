
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "xstd/h/sstream.h"
#include "xstd/Checksum.h"
#include "xstd/gadgets.h"
#include "base/AnyToString.h"
#include "base/BStream.h"
#include "runtime/IOBuf.h"

#include "csm/XmlTagParser.h"
#include "csm/oid2Url.h"
#include "csm/InjectIter.h"
#include "csm/EmbedContMdl.h"
#include "csm/ContentDbase.h"
#include "csm/cdbEntries.h"


/* CdbEntryPrnOpt */

CdbEntryPrnOpt::CdbEntryPrnOpt(): buf(0), injector(0), sizeMax(-1), 
	entryOff(-1), entryData(0) {
}


/* CdbEntry */

bool CdbEntry::Pour(const Area &image, bool divisible, CdbEntryPrnOpt &opt, bool &needMore) {
	if (image.size() <= 0) { // empty
		needMore = false;
		opt.entryOff = 0;
		return true;
	}

	if (!Should(opt.entryOff < image.size())) // internal error
		return false;

	Size remainingSize = Size(image.size()) - opt.entryOff;
	if (remainingSize > opt.sizeMax) { // body ends sooner
		if (!divisible)
			return false; // and we cannot divide
		remainingSize = opt.sizeMax;
	}

	const Size space = opt.buf->spaceSize();
	if (remainingSize > space) {
		needMore = true;
		if (space > 0) {
			opt.buf->append(image.data() + opt.entryOff, space);
			opt.entryOff += space;
			opt.sizeMax -= space;
		}
	} else {
		needMore = false;
		opt.buf->append(image.data() + opt.entryOff, remainingSize);
		opt.entryOff = 0;
		opt.sizeMax -= remainingSize;
	}

	return true;
}


/* CdbeBlob */

void CdbeBlob::image(const String &anImage) {
	theImage = anImage;
}

OBStream &CdbeBlob::store(OBStream &ol) const {
	return ol << theImage;
}

IBStream &CdbeBlob::load(IBStream &il) {
	return il >> theImage;
}

ostream &CdbeBlob::print(ostream &os) const {
	return os << theImage;
}

bool CdbeBlob::pour(CdbEntryPrnOpt &opt, bool &needMore) const {
	return Pour(theImage.area(0), false, opt, needMore);
}


/* CdbeText */

bool CdbeText::pour(CdbEntryPrnOpt &opt, bool &needMore) const {
	const Size mark = opt.buf->contSize();
	if (!Pour(theImage.area(0), true, opt, needMore))
		return false;
	if (opt.injector)
		opt.injector->inject(*opt.buf, 0, opt.buf->contSize() - mark);
	return true;
}


/* CdbeLink */

Size CdbeLink::size(CdbEntryPrnOpt &opt) const {
	return generateImage(opt.embed).size();
}

Size CdbeLink::meanSize() const {
	return Size(45); // roughly
}

OBStream &CdbeLink::store(OBStream &ol) const {
	return ol << contentCategory << origImage;
}

IBStream &CdbeLink::load(IBStream &il) {
	return il >> contentCategory >> origImage;
}

ostream &CdbeLink::print(ostream &os) const {
	return os << "[link:" << contentCategory << '@' << origImage << ']';
}

bool CdbeLink::pour(CdbEntryPrnOpt &opt, bool &needMore) const {
	CdbEntryPrnOpt::Embed &e = opt.embed;
	if (!Should(e.model && e.rng))
		return false;

	const Area image = generateImage(e);
	Should(image.size() > 0) &&
	Should(*image.data() == '/');

	if (!Pour(image, false, opt, needMore))
		return false;

	return true;
}

Area CdbeLink::generateImage(CdbEntryPrnOpt::Embed &e) const {
	Assert(e.model);

	const ObjId eid =
		e.model->embedCatOid(e.container, e.count, contentCategory);

	static char buf[8*1024];
	ofixedstream os(buf, sizeof(buf));
	Oid2UrlPath(eid, os);
	os.flush();
	Should(os);

	return Area::Create(buf, 0, Size(os.tellp()));
}


/* CdbePage */

CdbePage::CdbePage(): theDb(new ContentDbase) {
}

CdbePage::~CdbePage() {
	delete theDb;
}

Size CdbePage::size(CdbEntryPrnOpt &globOpt) const {
	CdbEntryPrnOpt opt = globOpt;
	Size sum = 0;
	int &pos = opt.embed.count;
	for (pos = 0; pos < theDb->count(); ++pos) {
		const CdbEntry *e = theDb->entry(pos);
		sum += e->size(opt);
	}
	return sum;
}

Size CdbePage::meanSize() const {
	return int(theDb->entrySizeMean() * theDb->count());
}

void CdbePage::add(CdbEntry *e) {
	theDb->add(e);
}

OBStream &CdbePage::store(OBStream &ol) const {
	theDb->store(ol);
	return ol;
}

IBStream &CdbePage::load(IBStream &il) {
	theDb->load(il);
	return il;
}

ostream &CdbePage::print(ostream &os) const {
	return theDb->print(os);
}

// based on CdbBodyIter::pourMiddle
bool CdbePage::pour(CdbEntryPrnOpt &globOpt, bool &needMore) const {
	CdbEntryPrnOpt &opt = globOpt;

	for (int &pos = opt.entryData; pos < theDb->count() && opt.sizeMax > 0; ++pos) {
		const CdbEntry *e = theDb->entry(pos);
		opt.embed.count = pos;

		if (!e->pour(opt, needMore))
			return false; // current entry will never fit

		if (needMore)
			return true;
	}

	needMore = false;
	return true;
}
