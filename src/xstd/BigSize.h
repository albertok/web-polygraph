
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_BIGSIZE_H
#define POLYGRAPH__XSTD_BIGSIZE_H

// BigSize can accommodate values up to 4611686016279904256 bytes
// for 4 byte integers, which is approximately 4 exabytes
// usually used for Size "accumulators"
class BigSize {
	public:
		static BigSize MB(int mb);
		static BigSize MBd(double mb);
		static BigSize Byted(double b);

	public:
		BigSize(int bytes = -1);

		bool operator ==(const BigSize &sz) const { return theCnt == sz.theCnt && theAcc == sz.theAcc; }
		bool operator !=(const BigSize &sz) const { return theCnt != sz.theCnt || theAcc != sz.theAcc; }
		bool operator  >(const BigSize &sz) const { return theCnt > sz.theCnt || (theCnt == sz.theCnt && theAcc > sz.theAcc); }
		bool operator >=(const BigSize &sz) const { return theCnt > sz.theCnt || (theCnt == sz.theCnt && theAcc >= sz.theAcc); }
		bool operator  <(const BigSize &sz) const { return theCnt < sz.theCnt || (theCnt == sz.theCnt && theAcc < sz.theAcc); }
		bool operator <=(const BigSize &sz) const { return theCnt < sz.theCnt || (theCnt == sz.theCnt && theAcc <= sz.theAcc); }

		// argument (void *) is ignored, '0' is assumed
		bool operator ==(void *) const { return !theCnt && !theAcc; }
		bool operator !=(void *) const { return theCnt || theAcc; }
		bool operator  >(void *) const { return theCnt > 0 || (theCnt == 0 && theAcc  > 0); }
		bool operator >=(void *) const { return theCnt > 0 || (theCnt == 0 && theAcc >= 0); }
		bool operator  <(void *) const { return theCnt < 0 || (theCnt == 0 && theAcc  < 0); }
		bool operator <=(void *) const { return theCnt < 0 || (theCnt == 0 && theAcc <= 0); }

		BigSize &operator +=(const BigSize &sz);
		BigSize &operator -=(const BigSize &sz);
		BigSize &operator *=(double f);
		double operator /=(const BigSize &sz) const;

		int byte() const; // will panic if overflow
		double byted() const;

		ostream &print(ostream &os) const;

	protected:
		void addBytes(int sz);
		void subBytes(int sz);

	public: /* log only */
		int theCnt; // counts the accumulated chunks
		int theAcc; // accumulates values in MAX_INT chunks
};


/* operators */

inline
BigSize operator +(BigSize sz1, BigSize sz2) { return sz1 += sz2; }

inline
BigSize operator -(BigSize sz1, BigSize sz2) { return sz1 -= sz2; }

inline
double operator /(BigSize sz1, BigSize sz2) { return sz1 /= sz2; }

inline
BigSize operator *(BigSize sz, double f) { return sz *= f; }

inline
BigSize operator /(BigSize sz, double f) { return sz *= 1/f; }

inline
BigSize operator +(BigSize sz) { return sz; }

inline
BigSize operator -(BigSize sz) { return BigSize::MB(0) - sz; }


inline
ostream &operator <<(ostream &os, const BigSize &sz) { return sz.print(os); }

#endif
