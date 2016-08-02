
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_ILOG_H
#define POLYGRAPH__BASE_ILOG_H

#include "xstd/h/iostream.h"

#include "xstd/Time.h"
#include "xstd/Size.h"
#include "xstd/NetDouble.h"
#include "xstd/BigSize.h"
#include "xstd/String.h"
#include "xstd/Array.h"
#include "xstd/Endian.h"
#include "xstd/NetAddr.h"
#include "base/Progress.h"

// some environments do not know better than #define these
#ifdef getc
#undef getc
#endif
#ifdef putc
#undef putc
#endif

namespace zlib {
class IFStream;
}

// every log entry has this prefix
class LogEntryPx {
	public:
		LogEntryPx(): theCat(-1), theTag(-1) {}

		bool good() const;
		operator void*() const { return theTag > 0 ? (void*)-1 : 0; }

		bool load(const void *const buf);
		ostream &print(ostream &os) const;

	public:
		static const int TheLoadSize = 2 * sizeof(int);

		Size theSize;    // size, including the header
		int theCat;      // logging category
		int theTag;      // logging tag
};

inline
ostream &operator <<(ostream &os, const LogEntryPx &px) {
	return px.print(os);
}


// binary log, input interface
class ILog {
	public:
		typedef void (Manip)(class ILog &);

	public:
		ILog();
		~ILog();

		void stream(const String &aFileName, istream *aStream);
		void stream(const ILog &log);
		const String &fileName() const { return theFileName; }
		istream::pos_type pos() const;
		bool fail() const;
		operator void*() const { return !fail() ? (void*)-1 : 0; }

		char getc() { char c; get(&c, 1); return c; }
		bool getb() { return getc() != 0; }
		int geti() { int x; return(geti(x)); }
		int geti(int &x) { get(&x, sizeof(x)); return x = ntohl(x); }
		int64_t geti64() { int64_t x; return geti64(x); }
		int64_t geti64(int64_t &x) { get(&x, sizeof(x)); return x = be64toh(x); }
		int geti(int *&xs, int &count); // returns count
		sockaddr_storage &geta(struct sockaddr_storage &a) { get(&a, SizeOf(a)); return a; }
		String &gets(String &s);

		LogEntryPx begEntry();
		void endEntry();

		const Progress &progress() const { return theProgress; }

	protected:
		void getHeader();
		bool get(void *const buf, const int len);
		void ignore(const int n);

	protected:
		istream *theStream;
		zlib::IFStream *theZStream;
		String theFileName;

		streampos theEntryEnd; // end of the current entry (last+1)
		LogEntryPx theCurPx;   // prefix of the current entry
		Progress theProgress;  // current "local time", #errs, etc.
};


/* logging of common types */

inline
ILog &operator >>(ILog &il, char &c) {
	c = il.getc();
	return il;
}

inline
ILog &operator >>(ILog &il, bool &b) {
	b = il.getb();
	return il;
}

inline
ILog &operator >>(ILog &il, int &x) {
	il.geti(x);
	return il;
}

inline
ILog &operator >>(ILog &il, int64_t &x) {
	il.geti64(x);
	return il;
}

inline
ILog &operator >>(ILog &il, double &x) {
	NetDouble nd;
	il >> nd.mnt >> nd.exp;
	x = nd;
	return il;
}

inline
ILog &operator >>(ILog &il, String &s) {
	il.gets(s);
	return il;
}

inline
ILog &operator >>(ILog &il, Time &t) {
	t.tv_sec = il.geti();
	t.tv_usec = il.geti();
	return il;
}

inline
ILog &operator >>(ILog &il, BigSize &bs) {
	return il >> bs.theAcc >> bs.theCnt;
}

inline
ILog &operator >>(ILog &il, Size &sz) {
	return il >> sz.theSize;
}

inline
ILog &operator >>(ILog &il, NetAddr &a) {
	struct sockaddr_storage ss;
	il.geta(ss);
	a = NetAddr(ss);
	return il;
}

inline
ILog &operator >>(ILog &il, ILog::Manip m) {
	m(il);
	return il; 
}

template <class Item>
inline
ILog &operator >>(ILog &log, Array<Item> &a) {
	const int cnt = log.geti();
	if (!log)
		return log;
	a.resize(cnt);
	for (int i = 0; i < cnt; ++i)
		log >> a[i];
	return log;
}

template <class Item>
inline
ILog &operator >>(ILog &log, Array<Item *> &a) {
	const int cnt = log.geti();
	if (!log)
		return log;
	for (int i = 0; i < a.count(); ++i)
		delete a[i];
	a.clear();
	a.resize(cnt);
	for (int i = log.geti(); log && i >= 0; log >> i) {
		Must(i < cnt);
		if (Should(!a[i]))
			a[i] = new Item;
		log >> *a[i];
	}
	return log;
}

#endif
