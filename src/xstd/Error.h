
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_ERROR_H
#define POLYGRAPH__XSTD_ERROR_H

#include "xstd/h/iosfwd.h"
#include "xstd/h/errno.h"

class String;

// errno wrapper
class Error {
	public:
		static const Error None() { return Error(); }
		static const Error Last();
		static const Error Last(const Error &err);
		static const Error LastExcept(const Error &err);
		static const Error Add(const char *str); // creates custom error

	protected:
		static const int TheCustomOff;

	public:
		Error(int aNo = 0): theNo(aNo) {}

		operator void*() const { return no() ? (void*)-1 : (void*)0; }
		bool operator ==(const Error &e) const { return theNo == e.theNo; }
		bool operator !=(const Error &e) const { return !(e == *this); }

		bool custom() const;

		int no() const { return theNo; }

		// use immediately; may return a pointer to shared memory
		const char *str() const;

		ostream &print(ostream &os) const { return print(os, str()); }

	protected:
		ostream &print(ostream &os, const char *str) const;

	protected:
		int theNo;		
};

inline
ostream &operator <<(ostream &os, const Error &err) {
	return err.print(os);
}

#endif
