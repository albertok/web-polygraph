
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_OLOG_H
#define POLYGRAPH__BASE_OLOG_H

#include "xstd/h/iosfwd.h"
#include "xstd/Time.h"
#include "xstd/Size.h"
#include "xstd/NetDouble.h"
#include "xstd/BigSize.h"
#include "xstd/String.h"
#include "xstd/NetAddr.h"
#include "xstd/Array.h"
#include "xstd/Endian.h"

// some environments do not know better than #define these
#ifdef getc
#	undef getc
#endif
#ifdef putc
#	undef putc
#endif

namespace zlib {
class OFStream;
}

// buffered binary output log
class OLog {
	public:
		typedef void (Manip)(class OLog &);

	public:
		OLog();
		virtual ~OLog();

		Size capacity() const { return theCapacity; }
		Size size() const { return theSize; }

		void stream(const String &aName, ostream *aStream);
		void capacity(Size aCap);

		void putc(char c) { put(&c, 1); }
		void putb(bool b) { putc((char)(b ? 1 : 0)); }
		void puti(int x) { x = htonl(x); put(&x, SizeOf(x)); }
		void puti(const int *x, int count);
		void puti64(int64_t x) { x = htobe64(x); put(&x, SizeOf(x)); }
		void puts(const char *s, Size size) { puti(size); if (size) put(s, size); }
		void puta(const struct sockaddr_storage &a) { put(&a, SizeOf(a)); }

		virtual void begEntry(int tag);
		void endEntry();

		void flush(Size maxSize); // flushes up-to max size
		void flush() { flush(theSize); }
		void close(); // flushes if needed

	protected:
		void resize(Size minCap);

		inline void put(const void *buf, Size size);
		void write(const char *const buf, const Size size);
		void overflow(const void *buf, Size size);

		virtual void putHeader();
		virtual void putTrailer();

	protected:
		ostream *theStream;
		zlib::OFStream *theZStream;
		String theName;

		char *theBuf;
		Size theCapacity;  // buffer space allocated
		Size theSize;      // buffer space used
		Size thePos;       // global offset

		char *theEntry;	   // start of the current entry
		int theEntryTag;   // tag saved until endEntry()

		Array<int> theDir; // directory (one entry per tag)
};


/* manipulators to begin and end logging of a named entry */

struct bege {
	bege(int t, int cat): tag(t | (cat << 16)) {}
	int tag;
};

inline void ende(OLog &log) { log.endEntry(); }


/* logging of common types */

inline
OLog &operator <<(OLog &ol, char c) {
	ol.putc(c);
	return ol;
}

inline
OLog &operator <<(OLog &ol, bool b) {
	ol.putb(b);
	return ol;
}

inline
OLog &operator <<(OLog &ol, int x) {
	ol.puti(x);
	return ol;
}

inline
OLog &operator <<(OLog &ol, const int64_t x) {
	ol.puti64(x);
	return ol;
}

inline 
OLog &operator <<(OLog &ol, double x) { 
	NetDouble nd(x);
	return ol << nd.mnt << nd.exp;
}

inline
OLog &operator <<(OLog &ol, const char *s) {
	ol.puts(s, strlen(s)+1);
	return ol;
}

inline
OLog &operator <<(OLog &ol, const String &s) {
	if (s)
		ol.puts(s.cstr(), s.len()+1);
	else
		ol.puts(0, 0);
	return ol;
}

inline
OLog &operator <<(OLog &ol, const Time &t) {
	ol.puti(t.tv_sec);
	ol.puti(t.tv_usec);
	return ol;
}

inline
OLog &operator <<(OLog &ol, const BigSize &bs) {
	return ol << bs.theAcc << bs.theCnt;
}

// XXX: should log either A or N
inline
OLog &operator <<(OLog &ol, const NetAddr &a) {
	ol.puta(a.addrN().sockAddr(a.port()));
	return ol;
}

inline
OLog &operator <<(OLog &ol, const bege &b) {
	ol.begEntry(b.tag);
	return ol;
}

inline
OLog &operator <<(OLog &ol, OLog::Manip m) {
	m(ol);
	return ol;
}

template <class Item>
inline
OLog &operator <<(OLog &ol, const Array<Item> &a) {
	ol.puti(a.count());
	for (int i = 0; i < a.count(); ++i)
		ol << a[i];
	return ol;
}

// store array of not-null pointers to items
template <class Item>
inline
OLog &operator <<(OLog &ol, const Array<Item *> &a) {
	ol.puti(a.count());
	for (int i = 0; i < a.count(); ++i) {
		if (a[i])
			ol << i << *a[i];
	}
	ol << -1;
	return ol;
}

/* inlined methods */

inline
void OLog::put(const void *buf, Size size) {
	if (size > 0 && theStream) {
		// just append if fits
		if (theSize + size <= theCapacity) {
			memcpy(theBuf + theSize, buf, size);
			theSize += size;
			thePos += size;
		} else {
			overflow(buf, size); // always uses put() recursively
		}
	}
}

#endif
