
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_BSTREAM_H
#define POLYGRAPH__BASE_BSTREAM_H

#include "xstd/h/iosfwd.h"
#include "xstd/h/netinet.h"    /* for ntoh and hton */
#include "xstd/Endian.h"

// perhaps operators for these should be moved into a separate file?
#include "xstd/Time.h"
#include "xstd/Size.h"
#include "xstd/NetDouble.h"
#include "xstd/BigSize.h"
#include "xstd/String.h"
#include "xstd/Array.h"

// a collection of streams that operate on binary objects
// storage format is platform-independent
// technically, each stream is a wrapper around corresponding
// iostream

// some environments do not know better than #define these
#ifdef getc
#undef getc
#endif
#ifdef putc
#undef putc
#endif

struct sockaddr_storage;
class NetAddr;

// base class for all binary streams
class BinIos {
	public:
		BinIos(): theOff(0) {}
		virtual ~BinIos();

		const String name() const { return theName; }
		Size offset() const { return theOff; }

	protected:
		String theName;
		Size theOff;
};

class IBStream: public virtual BinIos {
	public:
		typedef void (Manip)(class IBStream &);

	public:
		IBStream();
		~IBStream();

		void configure(istream *aStream, const String &aName);

		bool good() const { return theStream && theStream->good(); }

		char getc() { char c; get(&c, 1); return c; }
		bool getb() { return getc() != 0; }

		int geti() { int x; return geti(x); }
		int geti(int &x) { get(&x, SizeOf(x)); return x = ntohl(x); }

		unsigned int getui() { unsigned int x; return getui(x); }
		unsigned int getui(unsigned int &x) { get(&x, SizeOf(x)); return x = ntohl(x); }

		int64_t geti64() { int64_t x; return geti64(x); }
		int64_t geti64(int64_t &x) { get(&x, SizeOf(x)); return x = be64toh(x); }

		unsigned short getUsi() { unsigned short x; return getUsi(x); }
		unsigned short getUsi(unsigned short &x) { get(&x, SizeOf(x)); return x = ntohs(x); }
		struct sockaddr_storage &geta(struct sockaddr_storage &a);
		String &gets(String &s);

		// use to get raw strings only
		void get(void *buf, Size size);

		void skip(Size size);

		istream *stream() { return theStream; }

	protected:
		istream *theStream;
};

class OBStream: public virtual BinIos {
	public:
		typedef void (Manip)(class OBStream &);

	public:
		OBStream();
		~OBStream();

		void configure(ostream *aStream, const String &aName);

		bool good() const { return theStream && theStream->good(); }

		void putc(char c) { put(&c, 1); }
		void putb(bool b) { putc((char)(b ? 1 : 0)); }
		void puti(int x) { x = htonl(x); put(&x, SizeOf(x)); }
		void putui(unsigned int x) { x = htonl(x); put(&x, SizeOf(x)); }
		void puti64(int64_t x) { x = htobe64(x); put(&x, SizeOf(x)); }
		void putUsi(unsigned short x) { x = htons(x); put(&x, SizeOf(x)); }
		void puts(const String &s) { puti(s.len()); if (s.len()) put(s.data(), s.len()); }
		void puta(const struct sockaddr_storage &a);

		// use to store raw strings only
		void put(const void *buf, Size size);

		ostream *stream() { return theStream; }

	protected:
		ostream *theStream;
};

class BStream: public IBStream, public OBStream {
	public:
		void configure(iostream *aStream, const String &aName);
};

// manipulator allowing BStream << and >> operators for containers
template <class Container>
class ContainerStreamerT {
	public:
		explicit ContainerStreamerT(Container &aContainer): theContainer(aContainer) {}

		Container &theContainer;
};


/* operators for common types */

inline
IBStream &operator >>(IBStream &is, char &c) {
	c = is.getc();
	return is;
}

inline
IBStream &operator >>(IBStream &is, bool &b) {
	b = is.getb();
	return is;
}

inline
IBStream &operator >>(IBStream &is, int &x) {
	is.geti(x);
	return is;
}

inline
IBStream &operator >>(IBStream &is, unsigned int &x) {
	is.getui(x);
	return is;
}

inline
IBStream &operator >>(IBStream &is, int64_t &x) {
	is.geti64(x);
	return is;
}

inline
IBStream &operator >>(IBStream &is, unsigned short &x) {
	is.getUsi(x);
	return is;
}

inline
IBStream &operator >>(IBStream &is, double &x) {
	NetDouble nd;
	is >> nd.mnt >> nd.exp;
	x = nd;
	return is;
}

inline
IBStream &operator >>(IBStream &is, String &s) {
	is.gets(s);
	return is;
}

inline
IBStream &operator >>(IBStream &is, Time &t) {
	t.tv_sec = is.geti();
	t.tv_usec = is.geti();
	return is;
}

inline
IBStream &operator >>(IBStream &is, BigSize &bs) {
	return is >> bs.theAcc >> bs.theCnt;
}

inline
IBStream &operator >>(IBStream &is, Size &sz) {
	return is >> sz.theSize;
}

extern IBStream &operator >>(IBStream &is, NetAddr &a);

inline
IBStream &operator >>(IBStream &is, IBStream::Manip m) {
	m(is);
	return is; 
}

template <class Item>
inline
IBStream &operator >>(IBStream &is, Array<Item> &a) {
	const int cnt = is.geti();
	if (a.count()) { // use existing entries
		Assert(cnt <= a.count());
		for (int i = 0; is.good() && i < cnt; ++i)
			is >> a.item(i);
	} else {
		for (int i = 0; is.good() && i < cnt; ++i) {
			Item item;
			if ((is >> item).good())
				a << item;
		}
	}
	return is;
}

template <class Container>
inline
IBStream &operator >>(IBStream &is, ContainerStreamerT<Container> cs) {
	Container &container = cs.theContainer;
	container.clear();
	const int cnt = is.geti();
	for (int i = 0; is.good() && i < cnt; ++i) {
		typename Container::value_type item;
		if ((is >> item).good())
			container.push_back(item);
	}
	return is;
}

inline
OBStream &operator <<(OBStream &os, char c) {
	os.putc(c);
	return os;
}

inline
OBStream &operator <<(OBStream &os, bool b) {
	os.putb(b);
	return os;
}

inline
OBStream &operator <<(OBStream &os, int x) {
	os.puti(x);
	return os;
}

inline
OBStream &operator <<(OBStream &os, unsigned int x) {
	os.putui(x);
	return os;
}

inline
OBStream &operator <<(OBStream &os, const int64_t x) {
	os.puti64(x);
	return os;
}

inline
OBStream &operator <<(OBStream &os, unsigned short x) {
	os.putUsi(x);
	return os;
}

inline 
OBStream &operator <<(OBStream &os, double x) { 
	NetDouble nd(x);
	return os << nd.mnt << nd.exp;
}

inline
OBStream &operator <<(OBStream &os, const char *s) {
	const int len = strlen(s);
	os.puti(len);
	if (len)
		os.put(s, len);
	return os;
}

inline
OBStream &operator <<(OBStream &os, const String &s) {
	os.puts(s);
	return os;
}

inline
OBStream &operator <<(OBStream &os, const Time &t) {
	os.puti(t.tv_sec);
	os.puti(t.tv_usec);
	return os;
}

inline
OBStream &operator <<(OBStream &os, const BigSize &bs) {
	return os << bs.theAcc << bs.theCnt;
}

extern OBStream &operator <<(OBStream &os, const NetAddr &a);

inline
OBStream &operator <<(OBStream &os, OBStream::Manip m) {
	m(os);
	return os;
}

template <class Item>
inline
OBStream &operator <<(OBStream &os, const Array<Item> &a) {
	os.puti(a.count());
	for (int i = 0; i < a.count(); ++i)
		os << a[i];
	return os;
}

template <class Container>
inline
OBStream &operator <<(OBStream &os, const ContainerStreamerT<Container> cs) {
	const Container &container = cs.theContainer;
	os.puti(container.size());
	for (typename Container::const_iterator i = container.begin();
		i != container.end(); ++i)
		os << *i;
	return os;
}

// convenience wrapper to create ContainerStreamerT objects
template <class Container>
inline
ContainerStreamerT<Container> ContainerStreamer(Container &container) {
	return ContainerStreamerT<Container>(container);
}

#endif
