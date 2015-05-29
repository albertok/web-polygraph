
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/CompoundXactInfo.h"


CompoundXactInfo *CompoundXactInfo::TheFree = 0;


CompoundXactInfo *CompoundXactInfo::Create() {
	CompoundXactInfo *info = 0;
	if (TheFree) {
		info = TheFree;
		TheFree = TheFree->theNext;
		info->reset();
	} else
		info = new CompoundXactInfo();

	return info;
}

void CompoundXactInfo::Abandon(CompoundXactInfo *&info) {
	Should(!info->theNext);
	info->theNext = TheFree;
	TheFree = info;
	info = 0;
}

CompoundXactInfo::CompoundXactInfo() {
	reset();
}

CompoundXactInfo::~CompoundXactInfo() {
}

void CompoundXactInfo::reset() {
	reqSize = repSize = 0;
	startTime = lifeTime = Time();
	exchanges = 0;
	final = false;
	theNext = 0;
}
