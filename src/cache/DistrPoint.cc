
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/Client.h"
#include "cache/CacheEntry.h"
#include "cache/Cache.h"
#include "cache/DistrPoint.h"

Array<DistrPoint*> DistrPoint::TheWaits(1024);


DistrPoint::DistrPoint(Cache *aCache, CacheEntry *anEntry): 
	theCache(aCache), theEntry(anEntry),
	theReader(0), theWriter(0), theWaitIdx(-1), theToDo(0),
	gotReply(false) {
}

DistrPoint::~DistrPoint() {
	Assert(!theReader && !theWriter);
	if (theEntry)
		theEntry->theDistrPoint = 0;
	if (waiting())
		willNotCall();
}

void DistrPoint::addReader(CacheReader *r) {
	Assert(theEntry && !theReader);
	theReader = r;
	if (theReadySize >= 0)
		willCall(toPushData);
}

void DistrPoint::addWriter(CacheWriter *w) {
	Assert(theEntry && !theWriter);
	theWriter = w;
	// we do not know what the object size will be
	theReadySize = -1;
	theEntry->objSize(theReadySize);
}

void DistrPoint::delReader(CacheReader *r) {
	Assert(r && r == theReader);
	r->theDistrPoint = 0;
	theReader = 0;
	if (!theWriter)
		stop();
}

void DistrPoint::delWriter(CacheWriter *w) {
	Assert(w && w == theWriter);
	w->theDistrPoint = 0;
	theWriter = 0;
	if (!theReader)
		stop();
	// XXX: else should queue theReader->noteWriterLeft()
}

const RepHdr &DistrPoint::cachedRepHdr() const {
	Assert(hasRepHdr());
	return theRepHdr;
}

RepHdr &DistrPoint::repHdr() {
	Assert(!gotReply); // not a problem here, but clt should not call twice
	gotReply = true;
	return theRepHdr;
}


void DistrPoint::noteDataReady(Size readySize) {
	Assert(theReadySize <= readySize);
	theReadySize = readySize;
	willCall(toPushData);
}

void DistrPoint::mustProvideWriter() {
	Assert(theCache);
	Assert(theReader && !theWriter);
	Assert(theEntry);
	willCall(toProvideWriter);
}

void DistrPoint::provideWriter() {
	Assert(theCache);
	Assert(theCache->client());
	Assert(theReader && !theWriter);
	Assert(theEntry);
	if (!theCache->client()->fetch(theEntry->id(), this)) {
		Assert(!theWriter);
		theReader->noteWriterLeft(); // will call delReader
	}
	// it is unsafe to do anything here
}

void DistrPoint::pushData() {
	if (theReader)
		theReader->noteCacheReady();
	// it is unsafe to do anything here
}

void DistrPoint::callWriterLeft() {
	if (theReader)
		theReader->noteWriterLeft();
}

void DistrPoint::stop() {
	Assert(theEntry);
	Assert(!theReader && !theWriter);
	if (waiting())
		willNotCall();
	theEntry->stopDistributing(this);
	// stopDistributing destroys us
}

void DistrPoint::willCall(unsigned int toDo) {
	//cerr << here << "will do " << toDo << " at " << theWaitIdx << endl;
	if (toDo) {
		theToDo |= toDo;
		if (!waiting())
			theWaitIdx = WillCall(this);
	}
}

void DistrPoint::willNotCall() {
	Assert(waiting());
	WillNotCall(theWaitIdx);
	theToDo = 0;
	theWaitIdx = -1;
}

void DistrPoint::callWaiting() {
	//cerr << here << "doing " << theToDo << " at " << theWaitIdx << endl;
	Assert(waiting());
	theWaitIdx = -1;

	// one to-do call at a time!

	if (theToDo & toPushData) {
		theToDo &= ~toPushData;
		willCall(theToDo);
		pushData();
		return;
	} 

	if (theToDo & toProvideWriter) {
		theToDo &= ~toProvideWriter;
		willCall(theToDo);
		provideWriter();
		return;
	}

	if (theToDo & toCallWriterLeft) {
		theToDo &= ~toCallWriterLeft;
		willCall(theToDo);
		callWriterLeft();
		return;
	}

	Assert(false);
}

int DistrPoint::WillCall(DistrPoint *dp) {
	TheWaits.append(dp);
	return TheWaits.count()-1;
}

void DistrPoint::WillNotCall(int widx) {
	Assert(widx >= 0);
	if (widx < TheWaits.count())
		TheWaits[widx] = 0;
}

void DistrPoint::CallAll() {
	// It is probably safe to call all waits,
	// including those added during the loop.
	// Be fair, use SCAN approach, not "shortest seek"
	while (int count = TheWaits.count()) {
		for (int i = 0; i < count; ++i) {
			if (DistrPoint *dp = TheWaits[i])
				dp->callWaiting();
		}
		TheWaits.shift(count);
	}
}
