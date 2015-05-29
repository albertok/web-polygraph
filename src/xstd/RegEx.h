
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_REGEX_H
#define POLYGRAPH__XSTD_REGEX_H

#include "xstd/h/iosfwd.h"
#include "xstd/h/regex.h"
#include "xstd/Array.h"
#include "xstd/String.h"
#include "xstd/Size.h"


// a wrapper around POSIX 1003.2 regular expressions library
class RegEx: public regex_t {
	public:
		enum Flags { reExact = 1, reIgnoreCase = 2, reStartEnd = 4, 
			reLast = 8 };

		typedef regmatch_t Match;
		typedef Array<Match> Matches;

	protected:
		enum Kind { kdReMatch = reLast*2, kdStrStr = reLast*4, kdStrCmp = reLast*8 };

	public:
		static Match StartEnd(Size offset, Size size);

	public:
		RegEx();
		~RegEx();

		void configure(const String &image, const String &pattern, int flags = 0);

		operator void*() const { return valid() ? (void*)-1 : 0; }
		bool operator !() const { return !valid(); }

		bool valid() const;
		bool configured() const;

		bool exactMatching() const { return theFlags & reExact; }
		bool ignoringCase() const { return theFlags & reIgnoreCase; }
		int flags() const { return theFlags; }
		const String &image() const { return theImage; }
		const String &pattern() const { return thePattern; }

		bool match(const char *str, int flags = 0, int *errNo = 0) const;
		bool match(const char *str, Matches &matches, int flags = 0, int *errNo = 0) const;

		ostream &print(ostream &os) const { return os << theImage; }

	protected:
		bool hasReChars(const String &pattern) const;
		void confStrCmp();
		void confStrStr();
		void confReMatch();
		bool strCmp(const char *str) const;
		bool strStr(const char *str) const;
		bool reMatch(const char *str, Matches &matches, int flags) const;
		const char *strcaseStr(const char *str) const;

	protected:
		String theImage;
		String thePattern;
		int theFlags;
		mutable int theError;
		bool isCompiled;
};


#endif
