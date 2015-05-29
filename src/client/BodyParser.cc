
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "base/polyLogCats.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "client/CltXact.h"
#include "client/BodyParser.h"


BodyParser::BodyParser(): theOwner(0), isUsed(false) {
}

BodyParser::~BodyParser() {
	resetSelf();
}

void BodyParser::configure(CltXact *anOwner) {
	Should(!theOwner ^ !anOwner);
	theOwner = anOwner;
}

void BodyParser::reset() {
	resetSelf();
}

Size BodyParser::noteData(const ParseBuffer &data) {
	isUsed = true;
	return parse(data);
}

void BodyParser::noteLeftovers(const ParseBuffer &leftovers) {
	ReportError(errContentLeftovers);
	theOwner->noteContent(leftovers);
}

void BodyParser::noteOverflow(const ParseBuffer &buf) {
	ReportError(errHugeContentToken);
	theOwner->noteContent(buf);
}

int BodyParser::logCat() const {
	return lgcCltSide;
}

ostream &BodyParser::dumpContext(ostream &os, const char *ctxBeg, Size ctxSize) const {
	const Size prnSize = Min(ctxSize, Size(60));
	os.write(ctxBeg, prnSize);
	if (prnSize < ctxSize)
		os << "...";
	return os;
}

void BodyParser::resetSelf() {
	theOwner = 0;
	isUsed = false;
}
