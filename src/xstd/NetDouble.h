
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_NETDOUBLE_H
#define POLYGRAPH__XSTD_NETDOUBLE_H

// binary, fpoint-representation-independent storage for doubles
class NetDouble {
	public:
		NetDouble(): mnt(0), exp(0) {}
		NetDouble(double x);     // extracts components 

		operator double() const; // merges components

		inline bool operator ==(const NetDouble &d);
		inline bool operator !=(const NetDouble &d);

	public:
		int mnt; // mantissa
		int exp; // exponent
};

inline
bool NetDouble::operator ==(const NetDouble &d) {
	return mnt == d.mnt && exp == d.exp;
}

inline
bool NetDouble::operator !=(const NetDouble &d) {
	return mnt != d.mnt || exp != d.exp;
}

#endif
