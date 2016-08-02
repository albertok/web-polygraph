
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/HostMap.h"
#include "runtime/Viservs.h"


ViservIterator::ViservIterator(): theIdx(0) {
	sync();
}

bool ViservIterator::atEnd() const {
	return theIdx >= TheHostMap->iterationCount();
}

int ViservIterator::viserv() const {
	return theIdx;
}

HostCfg *ViservIterator::host() const {
	return TheHostMap->at(theIdx);
}

const NetAddr &ViservIterator::addr() const {
	return host()->theAddr;
}

ObjUniverse *ViservIterator::universe() const {
	return host()->theUniverse;
}

ViservIterator &ViservIterator::operator ++() {
	next();
	return *this;
}

void ViservIterator::next() {
	theIdx++;
	sync();
}

void ViservIterator::sync() {
	for (; !atEnd(); ++theIdx) {
		HostCfg *host = TheHostMap->at(theIdx);
		if (host && host->theUniverse)
			return;
	}	
}
