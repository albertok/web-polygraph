
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/os_std.h"
#include "xstd/h/iomanip.h"

#include "xstd/ZFStream.h"
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

// assume buffer size is at least TheLoadSize
// XXX: we should log lgEnd/lgcEnd instead of using current constants!
bool LogEntryPx::load(const void *const buf) {
	const int *const ibuf = reinterpret_cast<const int *>(buf);
	theSize = ntohl(ibuf[0]);
	const int h = ntohl(ibuf[1]);
	theTag = (h << 16) >> 16;
	theCat = h >> 16; // XXX: assumes sizeof(int) == 32
	return *this;
}

ostream &LogEntryPx::print(ostream &os) const {
	return os
		<< "tag: " << theCat << '/' << theTag 
		<< " size: " << theSize;
	
}

/* ILog */

ILog::ILog(): theStream(0), theZStream(0), theEntryEnd(-1) {
}

ILog::~ILog() {
	delete theZStream;
}

void ILog::stream(const String &aFileName, istream *aStream) {
	theFileName = aFileName;
	theStream = aStream;
	getHeader();
}

void ILog::stream(const ILog &log) {
	log.theStream->clear();
	log.theStream->seekg(0, ios::beg);
	stream(log.theFileName, log.theStream);
}

// loads prefix of the current entry
// transparently loads progress entries
LogEntryPx ILog::begEntry() {
	theEntryEnd = pos();
	char buf[LogEntryPx::TheLoadSize];
	while (get(buf, LogEntryPx::TheLoadSize)) {
		if (theCurPx.load(buf) && !theCurPx.good()) {
			const streampos startOff = theEntryEnd;
			clog << fileName() << ':' << theEntryEnd 
				<< ": warning: corrupted log entry: " << theCurPx << endl;

			do {
				theEntryEnd += 1;
				memmove(buf, buf + 1, LogEntryPx::TheLoadSize - 1);
				buf[LogEntryPx::TheLoadSize - 1] = getc();
			} while (!fail() && !(theCurPx.load(buf) &&
				theCurPx.good() &&
				theCurPx.theTag == lgProgress &&
				theCurPx.theSize < 1024));
			if (fail())
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
	const int nextEntryOffset = theEntryEnd - pos();
	ignore(nextEntryOffset);
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

	const int sver = 26;       // supported version
	const int cver = geti();   // current log version
	const int rver = geti();   // required min version to support
	int extraHdrSz = geti();   // size of extra headers

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

	const int zlibHdrSz = 32;
	bool doCompression = false;
	if (extraHdrSz >= zlibHdrSz) {
		char buf[zlibHdrSz + 1];
		get(buf, zlibHdrSz);
		buf[zlibHdrSz] = '\0';
		extraHdrSz -= zlibHdrSz;

		if (!strcmp(buf, "zlib")) {
			if (!zlib::Supported) {
				cerr << theFileName << ": log file is "
					"compressed with zlib" << endl <<
					"this program was built without zlib "
					"support" << endl << xexit;
			}
			doCompression = true;
		} else {
			cerr << theFileName << ": log file is compressed with "
				<< buf << endl << "this program does not "
				"support it" << endl << xexit;
		}
	}

	ignore(extraHdrSz);
	if (doCompression)
		theZStream = new zlib::IFStream(*theStream);
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

bool ILog::get(void *const buf, const int len) {
	char *const cbuf = reinterpret_cast<char *>(buf);
	int count;
	if (theZStream)
		count = theZStream->read(cbuf, len);
	else {
		theStream->read(cbuf, len);
		count = theStream->gcount();
	}
	return count == len;
}

bool ILog::fail() const {
	return theZStream ? theZStream->fail() : theStream ? theStream->fail() :
		true;
}

istream::pos_type ILog::pos() const {
	return theZStream ? theZStream->pos() : theStream ? theStream->tellg() :
		istream::pos_type(-1);
}

void ILog::ignore(const int n) {
	Must(n >= 0);
	if (theZStream)
		theZStream->ignore(n);
	else
		theStream->ignore(n);
}
