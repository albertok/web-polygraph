
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HTTPVERSION_H
#define POLYGRAPH__RUNTIME_HTTPVERSION_H

class HttpVersion {
	public:
		HttpVersion(): theMajor(-1), theMinor(-1) {}
		HttpVersion(int aMajor, int aMinor): theMajor(aMajor), theMinor(aMinor) {}

		void reset() { theMajor = theMinor = -1; };

		bool known() const { return theMajor >= 0 && theMinor >= 0; }
		int vMajor() const { return theMajor; }
		int vMinor() const { return theMinor; }

		inline bool operator <(const HttpVersion &v) const;
		inline bool operator <=(const HttpVersion &v) const;

	protected:
		int theMajor;
		int theMinor;
};

inline
bool HttpVersion::operator < (const HttpVersion &v) const {
	return theMajor < v.theMajor ||
		(theMajor == v.theMajor && theMinor < v.theMinor);
}

inline
bool HttpVersion::operator <=(const HttpVersion &v) const {
	return theMajor < v.theMajor ||
		(theMajor == v.theMajor && theMinor <= v.theMinor);
}

#endif
