
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_AUTHSTAT_H
#define POLYGRAPH__BASE_AUTHSTAT_H

#include "base/AuthPhaseStat.h"
#include "base/LogObj.h"

// authIng and authEd response time/size stat for each scheme
class AuthStat: public LogObj {
	public:
		AuthStat();

		void reset();

		virtual OLog &store(OLog &log) const;
		virtual ILog &load(ILog &);
		bool sane() const;

		void authIngAll(TmSzStat &all) const;
		void authEdAll(TmSzStat &all) const;

		inline void recordAuthIng(const AuthPhaseStat::Scheme scheme, const Time &tm, const Size sz);
		inline void recordAuthEd(const AuthPhaseStat::Scheme scheme, const Time &tm, const Size sz);

		inline const TmSzStat &getAuthIng(const AuthPhaseStat::Scheme scheme) const;
		inline const TmSzStat &getAuthEd(const AuthPhaseStat::Scheme scheme) const;

		AuthStat &operator +=(const AuthStat &s);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		AuthPhaseStat theAuthIng;
		AuthPhaseStat theAuthEd;
};

/* inlined methods */

inline
void AuthStat::recordAuthIng(const AuthPhaseStat::Scheme scheme, const Time &tm, const Size sz) {
	theAuthIng.record(scheme, tm, sz);
}

inline
void AuthStat::recordAuthEd(const AuthPhaseStat::Scheme scheme, const Time &tm, const Size sz) {
	theAuthEd.record(scheme, tm, sz);
}

inline
const TmSzStat &AuthStat::getAuthIng(const AuthPhaseStat::Scheme scheme) const {
	return theAuthIng.get(scheme);
}

inline
const TmSzStat &AuthStat::getAuthEd(const AuthPhaseStat::Scheme scheme) const {
	return theAuthEd.get(scheme);
}

#endif
