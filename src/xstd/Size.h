
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_SIZE_H
#define POLYGRAPH__XSTD_SIZE_H

#include "xstd/h/iostream.h"

// reduces size convertion problems
// note: use BigSize for values of 2GB and larger
class Size {
	public:
		// XXX: Too small for many purposes.
		// XXX: Too unstable for binary logging purposes.
		typedef int size_type;

		static Size KB(int n) { return Size(n * 1024); }
		static Size MB(int n) { return Size::KB(n * 1024); }
		static Size Byte(int n) { return Size(n); }
		static Size Bit(int n);

	public:
		Size(): theSize(-1) {}
		Size(int bytes): theSize(bytes) {}

		bool known() const { return theSize >= 0; }

		Size operator ++() { return theSize++; }
		Size operator ++(int) { return theSize++; }
		Size operator += (Size sz) { return theSize += sz; }
		Size operator -= (Size sz) { return theSize -= sz; }
		Size operator *= (Size sz) { return theSize *= sz; }
		Size operator /= (Size sz) { return theSize /= sz; }
		Size operator %= (Size sz) { return theSize %= sz; }

		int byte() const { return theSize; }
		operator int() const { return byte(); }

		Size &operator =(int bytes) { theSize = bytes; return *this; }

		ostream &print(ostream &os) const;

	public: /* for logging only */
		size_type theSize;
};


/* operators */

inline
Size operator +(Size sz1, Size sz2) { return sz1 += sz2; }

inline
Size operator -(Size sz1, Size sz2) { return sz1 -= sz2; }

inline
double operator /(Size sz1, Size sz2) { return sz1 /= sz2; }

inline
Size operator *(Size sz, int f) { return sz *= f; }

inline
Size operator /(Size sz, int f) { return sz /= f; }

inline
ostream &operator <<(ostream &os, const Size &sz) { return sz.print(os); }

#define SizeOf(obj) ((Size)sizeof(obj))

#endif
