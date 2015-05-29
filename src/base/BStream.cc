
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"

#include "xstd/NetAddr.h"
#include "base/BStream.h"


/* BinIos */

BinIos::~BinIos() {
}

/* IBStream */

IBStream::IBStream(): theStream(0) {
}

IBStream::~IBStream() {
}

void IBStream::configure(istream *aStream, const String &aName) {
	Assert(!theStream && aStream);
	theStream = aStream;
	theName = aName;
}

String &IBStream::gets(String &s) {
	const int len = geti();
	Must(len >= 0);
	char *buf = s.alloc(len+1);
	if (len)
		get(buf, len);
	return s;
}

struct sockaddr_storage &IBStream::geta(struct sockaddr_storage &a) {
	get(&a, SizeOf(a));
	return a;
}

void IBStream::get(void *buf, Size size) {
	Assert(theStream);
	theStream->read((char*)buf, size);
	theOff += size;
}

void IBStream::skip(Size size) {
	theStream->seekg((streamoff)size, ios::cur);
	theOff += size;
}

IBStream &operator >>(IBStream &is, NetAddr &a) {
	struct sockaddr_storage ss;
	is.geta(ss);
	a = NetAddr(ss);
	is.gets(a.rawAddrA());
	return is;
}


/* OBStream */

OBStream::OBStream(): theStream(0) {
}

OBStream::~OBStream() {
}

void OBStream::configure(ostream *aStream, const String &aName) {
	Assert(!theStream && aStream);
	theStream = aStream;
	theName = aName;
}

void OBStream::put(const void *buf, Size size) {
	Assert(theStream);
	theStream->write((const char*)buf, size);
	theOff += size;
}

void OBStream::puta(const struct sockaddr_storage &a) {
	put(&a, SizeOf(a));
}

void BStream::configure(iostream *aStream, const String &aName) {
	IBStream::configure(aStream, aName);
	OBStream::configure(aStream, aName);
}

OBStream &operator <<(OBStream &os, const NetAddr &a) {
	os.puta(a.addrN().sockAddr(a.port()));
	os << a.rawAddrA();
	return os;
}
