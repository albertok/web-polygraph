
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iostream.h"
#include "xstd/h/string.h"

#include "xstd/Assert.h"
#include "xstd/String.h"
#include "xstd/gadgets.h"


/* StrBuf */

void StrBuf::alloc(int aSize) {
	theBuf = new char[theSize = aSize];
}

void StrBuf::set(const char *aBuf, int aSize) {
	Assert(!theUseLvl);
	//cerr << here << "new string: " << aSize << ' ' << aBuf << endl;
	delete[] theBuf;
	alloc(aSize);
	memcpy(theBuf, aBuf, aSize);
}

StrBuf *StrBuf::clone() const {
	StrBuf *sb = new StrBuf;
	sb->set(theBuf, theSize);
	return sb;
}

void StrBuf::realloc(int newSize) {
	if (newSize > theSize) {
		char *buf = theBuf;
		const int sz = theSize;
		theBuf = 0;
		alloc(newSize);
		if (buf)
			memcpy(theBuf, buf, sz);
		delete[] buf;
	}
}


/* String */

String::String(const char *s, int len): theBuf(0) {
	Assert(s);
	Assert(len >= 0);

	if (len > 0) {
		theBuf = new StrBuf;
		theBuf->alloc(len+1);
		memcpy(theBuf->theBuf, s, len);
		theBuf->theBuf[len] = '\0';
		theBuf->use();
	}
}

String::String(const Area &a): theBuf(0) {
	set(String(a.data(), a.size()));
}

String::String(const std::string &s): theBuf(0) {
	set(String(s.data(), s.length()));
}

void String::set(const char *s) {
	if (theBuf)
		theBuf = theBuf->abandon();

	if (s) {
		if (const int l = strlen(s)) {
			theBuf = new StrBuf;
			theBuf->set(s, l+1);
			theBuf->use();
		}
	}
}

void String::set(const String &s) {
	if (theBuf == s.theBuf)
		return; // either "copy to self" or "0 = 0"

	if (theBuf)
		theBuf->abandon();

	if ((theBuf = s.theBuf))
		theBuf->use();
}

char *String::alloc(int size) {
	if (theBuf)
		theBuf->abandon();
	theBuf = new StrBuf;
	theBuf->alloc(size);
	theBuf->use();
	return theBuf->theBuf;
}

void String::stretch(int newSize) {
	if (!theBuf || theBuf->theSize < newSize) {
		if (theBuf)
			theBuf->realloc(newSize);
		else
			alloc(newSize);
	}
}

// clone a private copy if we are using shared memory
// (copy-on-write or lazy copying)
void String::aboutToMod() {
	if (theBuf && theBuf->theUseLvl > 1) {
		StrBuf *sb = theBuf;
		theBuf = sb->clone();
		theBuf->use();
		sb->abandon();
	}
}

const char *String::chr(char c) const {
	return theBuf ? strchr(cstr(), c) : 0;
}

const char *String::rchr(char c) const {
	return theBuf ? strrchr(cstr(), c) : 0;
}

const char *String::str(const char *s) const {
	return theBuf ? strstr(cstr(), s) : 0;
}

const char *String::str(const String &s) const {
	return str(s.cstr());
}

String String::operator ()(int min, int maxp1) const {
	maxp1 = Min(maxp1, len());
	const int len = maxp1 - min;
	Assert(len >= 0);
	return String(data() + min, len);
}

Area String::area(int offset, int size) const {
	if (offset == 0 && size == npos && len() == 0)
		return Area::Empty();
	Assert(offset < len() || size == 0);
	return Area::Create(data(), offset, Min(size, len()-offset));
}

int String::cmp(const String &s) const {
	return strcmp(cstr(), s.cstr());
}

int String::cmp(const String &s, int len) const {
	return strncmp(cstr(), s.cstr(), len);
}

int String::cmp(const char *s) const {
	return strcmp(cstr(), s);
}

int String::cmp(const char *s, int size) const {
	return strncmp(cstr(), s, size);
}

bool String::startsWith(const String &s) const {
	return s.prefixOf(this->cstr());
}

bool String::prefixOf(const char *buf) const {
	if (const int l = len())
		return strncmp(theBuf->theBuf, buf, l) == 0;
	else
		return true; // empty string is always a prefix?
}

bool String::casePrefixOf(const char *buf, int size) const {
	if (const int l = len())
		return l <= size && !strncasecmp(theBuf->theBuf, buf, l);
	else
		return true; // empty string is always a prefix?
}

bool String::endsWith(const String &s) const {
	return endsWith(s.cstr(), s.len());
}

bool String::endsWith(const char *buf, int size) const {
	if (const int l = len())
		return size <= l && !strncmp(theBuf->theBuf + l-size, buf, size);
	else
		return false;
}

bool String::caseEndsWith(const char *buf, int size) const {
	if (const int l = len())
		return size <= l && !strncasecmp(theBuf->theBuf + l-size, buf, size);
	else
		return false;
}

int String::find(char c, int offset) const {
	if (!Should(offset <= len()))
		return npos;

	if (const char *p = strchr(cstr() + offset, c))
		return p - cstr();
	else
		return npos;
}

int String::find(const Area &area, int offset) const {
	if (!area)
		return npos;

	for (const int maxo = len() - area.size(); offset <= maxo; ++offset) {
		offset = find(area[0], offset);
		if (maxo < offset)
			break;
		if (strncmp(data()+offset, area.data(), area.size()) == 0)
			return offset;
	}
	return npos;
}

int String::hash() const {
	const int step = Max(1, len() / 16);
	unsigned int res = 0;
	for (int i = 0; i < len(); i += step)
		res = res*33U + data()[i];

	return Max(1, abs((int)res));
}

// adds 0 after appending if not already there
void String::append(const char *buf, int size) {
	Assert(buf || !size);

	aboutToMod();

	const bool addTerm = !size || buf[size-1] != (char)0;
	const int l = len();

	stretch(l + size + (int)addTerm);

	Assert(theBuf);
	if (size)
		memcpy(theBuf->theBuf+l, buf, size);
	theBuf->theBuf[len()] = '\0';
}

String &String::operator +=(const String &s) {
	append(s.data(), s.len());
	return *this;
}

String &String::operator +=(char c) {
	append(&c, 1);
	return *this;
}

String String::operator +(char c) const {
	String s(*this);
	s += c;
	return s;
}

/* operators on strings */

String operator +(const String &s1, const String &s2) {
	String s(s1);
	s += s2;
	return s;
}

String Stream2String(ostringstream &os) {
	const std::string s = os.str();
	const String res = s.size() > 0 ?
		String(s.data(), s.size()) : String();
	streamFreeze(os, false);
	return res;
}


/* StrIter */

StrIter::StrIter(const String &aStr, char aDel): 
	theStr(aStr), theTokBeg(0), theTokEnd(0), theDel(aDel) {
	if (theStr) {
		theTokBeg = theTokEnd = theStr.data();
		moveEnd();
	}
}

String StrIter::token() const {
	return String(tokBeg(), tokLen());
}

void StrIter::next() {
	if (theTokEnd < theStr.data() + theStr.len()) {
		theTokBeg = ++theTokEnd;
		moveEnd();
	} else {
		theTokBeg = theTokEnd;
	}
}

void StrIter::moveEnd() {
	while (theTokEnd < theStr.data() + theStr.len() && *theTokEnd != theDel)
		++theTokEnd;
}
