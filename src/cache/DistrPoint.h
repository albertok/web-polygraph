
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CACHE_DISTRPOINT_H
#define POLYGRAPH__CACHE_DISTRPOINT_H

#include "xstd/Array.h"
#include "runtime/httpHdrs.h"

class Cache;
class CacheEntry;
class CacheReader;
class CacheWriter;

// rendezvous point for object readers (serving side) and
// object writers (requesting side)

class DistrPoint {
	public:
		static void CallAll(); // activates all waiting actions

	protected:
		typedef enum { toPushData = 1, toProvideWriter = 2,
			toCallWriterLeft = 4 } ToDo;

	public:
		DistrPoint(Cache *aCache, CacheEntry *anEntry);
		~DistrPoint();

		Cache *cache() { return theCache; }
		CacheEntry *entry() { return theEntry; }
		Size readySize() const { return theReadySize; }
		const CacheReader *reader() const { return theReader; }
		const CacheWriter *writer() const { return theWriter; }

		void addReader(CacheReader *r);
		void addWriter(CacheWriter *w);
		void delReader(CacheReader *r);
		void delWriter(CacheWriter *w);

		// called by writer/reader to supply/read reply headers
		const RepHdr &cachedRepHdr() const; // for reader
		RepHdr &repHdr();             // for writer
		bool hasRepHdr() const { return gotReply; }

		// called by the writer (or reader on a hit)
		void noteDataReady(Size readySize);

		// called by cache to add writer via WillCall
		void mustProvideWriter();

	protected:
		static int WillCall(DistrPoint *dp);
		static void WillNotCall(int widx);

	protected:
		void willCall(unsigned int toDo);
		void willNotCall();
		bool waiting() const { return theWaitIdx >= 0; }
		void callWaiting();

		void pushData();
		void provideWriter();
		void callWriterLeft();
		void stop();

	protected:
		static Array<DistrPoint*> TheWaits; // postponed "recursive" calls

	protected:
		Cache *theCache;
		CacheEntry *theEntry;
		CacheReader *theReader; // XXX: only one reader is supported!
		CacheWriter *theWriter;

		RepHdr theRepHdr;       // cached values from the writer

		Size theReadySize;      // object [content] size

		int theWaitIdx;         // reservation with TheWaits
		unsigned int theToDo;   // waiting actions

		bool gotReply;			// writer cached reply values in theRepHdr
};

#endif
