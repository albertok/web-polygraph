
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/os_std.h"

#include "xstd/gadgets.h"
#include "base/ILog.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"


/* LogEntryPx */

// XXX: we should log lgEnd/lgcEnd instead of using current constants!
bool LogEntryPx::good() const {
	return
		0 <= theCat && theCat < lgcEnd &&
		0 <= theTag && theTag < lgEnd &&
		0 < theSize && theSize <= 10*1024*1024;
}

// XXX: we should log lgEnd/lgcEnd instead of using current constants!
ILog &LogEntryPx::load(ILog &il) {
	int h;
	il >> theSize >> h;
	theTag = (h << 16) >> 16;
	theCat = h >> 16; // XXX: assumes sizeof(int) == 32
	return il;
}

ostream &LogEntryPx::print(ostream &os) const {
	return os
		<< "tag: " << theCat << '/' << theTag 
		<< " size: " << theSize;
	
}

/* ILog */

ILog::ILog(): theStream(0), theEntryEnd(-1) {
}

ILog::~ILog() {
}

void ILog::stream(const String &aFileName, istream *aStream) {
	theFileName = aFileName;
	theStream = aStream;
	getHeader();
}

// loads prefix of the current entry
// transparently loads progress entries
LogEntryPx ILog::begEntry() {
	theEntryEnd = theStream->tellg();
	while (theCurPx.load(*this)) {

		if (theCurPx && !theCurPx.good()) {
			const streampos startOff = theEntryEnd;
			clog << fileName() << ':' << theEntryEnd 
				<< ": warning: corrupted log entry: " << theCurPx << endl;

			do {
				theEntryEnd += 1;
				endEntry();
				theCurPx.load(*this);
			} while (*theStream && !(theCurPx.theTag == lgProgress && theCurPx.theSize > 0 && theCurPx.theSize < 1024));
			if (!*theStream)
				break;

			clog << fileName() << ':' << theEntryEnd
				<< ": maybe recovered after skipping " << (theEntryEnd-startOff)
				<< " bytes: " << theCurPx << endl;
		}

		theEntryEnd += theCurPx.theSize;
		if (theCurPx.theTag != lgProgress)
			return theCurPx;
		theProgress.load(*this);
		endEntry();
	}
	theCurPx = LogEntryPx();
	return theCurPx;
}

void ILog::endEntry() {
	theStream->seekg(theEntryEnd);
	theCurPx = LogEntryPx();
}

void ILog::getHeader() {
	Assert(theStream);

	// check magic
	if (geti() != lgMagic1 || geti() != lgMagic2 || geti() != 0) {
		if (!*theStream || theStream->bad())
			cerr << theFileName << ": read error; " << Error::Last() << endl;
		else
			cerr << theFileName << ": unknown log file format" << endl;
		exit(-2);
	}

	const int sver = 21;       // supported version
	const int cver = geti();   // current log version
	const int rver = geti();   // required min version to support
	const int skip = geti();   // size of extra headers

	if (sver < rver) {
		cerr << theFileName << ": log version " << cver << endl;
		cerr << theFileName << ": requires support for log version " << rver << " or higher" << endl;
		cerr << theFileName << ": this program supports version " << sver << endl;
		exit(-2);
	}

	if (sver != cver) {
		cerr << theFileName << ": log version " << cver << endl;
		cerr << theFileName << ": this program supports version " << sver << endl;
		cerr << theFileName << ": continuing at your own risk..." << endl;
		sleep(3);
	}

	theStream->ignore(skip);
}

int ILog::geti(int *&xs, int &count) {
	Must(geti(count) >= 0);
	xs = new int[count];
	for (int i = 0; i < count; ++i)
		geti(xs[i]);
	return count;
}

String &ILog::gets(String &s) {
	const int sz = geti();
	if (Should(sz >= 0)) {
		if (sz > 0) {
			char *buf = s.alloc(sz);
			get(buf, sz);
			return s;
		}
	}
	s = String();
	return s;
}
