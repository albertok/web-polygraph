
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "runtime/PageInfo.h"


PageInfo *PageInfo::TheFree = 0;


PageInfo *PageInfo::Create() {
	PageInfo *info = 0;
	if (TheFree) {
		PageInfo *next = TheFree->theNext;
		info = TheFree;
		TheFree = next;
		info->reset();
	} else {
		info = new PageInfo();
	}
	return Use(info);
}

PageInfo *PageInfo::Use(PageInfo *info) {
	if (!Should(info))
		info = Create();
	info->use();
	return info;
}

void PageInfo::Abandon(PageInfo *&info) {
	if (Should(info->used())) {
		--info->theUseLevel;
		if (!info->used()) {
			Should(!info->theNext);
			info->theNext = TheFree;
			TheFree = info;
		}
	}
	info = 0;
}

PageInfo::PageInfo() {
	reset();
}

PageInfo::~PageInfo() {
	Should(!used());
}

void PageInfo::reset() {
	size = 0;
	start = lifeTime = Time();
	useMax = theUseLevel = 0;
	theNext = 0;
}

bool PageInfo::used() const {
	return theUseLevel > 0;
}

bool PageInfo::loneUser() const {
	return theUseLevel == 1;
}

void PageInfo::use() {
	if (!++theUseLevel)
		start = TheClock;
	if (theUseLevel > useMax)
		useMax = theUseLevel;
}
