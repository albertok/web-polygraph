
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_STRING_H
#define POLYGRAPH__XSTD_STRING_H

#include <limits.h>
#include "xstd/h/iosfwd.h"
#include "xstd/h/sstream.h"
#include "xstd/Area.h"

// memory storage to help with copy-on-write support
class StrBuf {
	public:
		StrBuf(): theBuf(0), theSize(0), theUseLvl(0) {}
		~StrBuf() { delete[] theBuf; }

		void set(const char *aBuf, int aSize);
		void alloc(int aSize);
		void realloc(int newSize);

		StrBuf *use() { theUseLvl++; return this; }
		StrBuf *abandon() { if (!--theUseLvl) delete this; return 0; }
		
		StrBuf *clone() const;

	public:
		char *theBuf;
		int theSize;
		mutable int theUseLvl;
};

// a simple string class; to be improved later if needed
class String {
	public:
		static const int npos = INT_MAX;

	public:
		String(): theBuf(0) {}
		String(const String &s): theBuf(0) { set(s); }
		String(const char *s): theBuf(0) { set(s); }
		String(const char *s, int aLen);
		explicit String(const Area &a);
		explicit String(const std::string &s);
		~String() { set(0); }

		String &operator =(const String &s) { set(s); return *this; }
		String &operator =(const char *s) { set(s); return *this; }

		const char *cstr() const { /* terminate(); */ return theBuf ? theBuf->theBuf : ""; }
		const char *data() const { return theBuf ? theBuf->theBuf : 0; }

		//int size() const { return theBuf ? theBuf->theSize : 0; }
		int len() const { return theBuf ? theBuf->theSize - 1 : 0; }
		const char *chr(char c) const;
		const char *rchr(char c) const;
		const char *str(const char *s) const;
		const char *str(const String &s) const;
		int cmp(const String &s) const;
		int cmp(const String &s, int len) const;
		int cmp(const char *s) const;
		int cmp(const char *s, int len) const;
		bool startsWith(const String &s) const;
		bool prefixOf(const char *buf) const;
		bool casePrefixOf(const char *buf, int l) const;
		bool endsWith(const String &s) const;
		bool endsWith(const char *buf, int l) const;
		bool caseEndsWith(const char *buf, int l) const;

		operator void*() const { return len() ? (void*)-1 : (void*)0; }
		bool operator !() const { return len() == 0; }
		bool operator <(const String &s) const { return cmp(s) < 0; }

		int find(const Area &area, int offset = 0) const;
		int find(char c, int offset = 0) const;

		char operator [](int idx) const { return theBuf->theBuf[idx]; }
		char &operator [](int idx) { aboutToMod(); return theBuf->theBuf[idx]; }
		char operator *() const { return *theBuf->theBuf; }

		String operator +(char c) const;

		char last() const { return (*this)[len()-1]; }

		int hash() const; // cheap and lame

		// substring: [min, maxp1)
		String operator ()(int min, int maxp1) const;
		Area area(int offset, int size = npos) const;

		// append
		void append(const char *buf, int size); // also adds 0 if not there
		String &operator +=(const String &s);
		String &operator +=(char c);

		// case sensitive
		bool operator ==(const String &s) const { return theBuf == s.theBuf ||
			(len() == s.len() && cmp(s) == 0); }
		bool operator !=(const String &s) const { return !(*this == s); }
		bool operator ==(const char *s) const { return cmp(s) == 0; }
		bool operator !=(const char *s) const { return !(*this == s); }

		// low level access to internal buffer for reading, avoid
		char *alloc(int size);

	protected:
		String(StrBuf &aBuf): theBuf(&aBuf) { theBuf->use(); }

		void set(const char *s);
		void set(const String &s);
		void stretch(int newSize);
		void aboutToMod();

	protected:
		StrBuf *theBuf;
};

inline ostream &operator <<(ostream &os, const String &s) {
	return s ? os.write(s.data(), s.len()) : os;
}

// slow but convenient
extern String operator +(const String &s1, const String &s2);

extern String Stream2String(ostringstream &os);

// a strtok() replacement
class StrIter {
	public:
		StrIter(const String &aStr, char aDel);

		bool eos() const { return !theTokBeg || !*theTokBeg; }
		operator void *() const { return eos() ? 0 : (void*)-1; }
		
		const char *tokBeg() const { return theTokBeg; }
		const char *tokEnd() const { return theTokEnd; }
		int tokLen() const { return tokEnd() - tokBeg(); }
		String token() const;

		StrIter &operator ++() { next(); return *this; }

	protected:
		void next();
		void moveEnd();

	protected:
		const String &theStr;
		const char *theTokBeg;
		const char *theTokEnd;
		const char theDel;    // delimiter
};

#endif
