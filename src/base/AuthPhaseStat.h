
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_AUTHPHASESTAT_H
#define POLYGRAPH__BASE_AUTHPHASESTAT_H

#include "base/TmSzStat.h"
#include "base/LogObj.h"

// auth response time/size stat for each scheme
class AuthPhaseStat: public LogObj {
	public:
		enum Scheme {
			sNone,
			sBasic,
			sNtlm,
			sNegotiate,
			sFtp,
			sSocksUserPass,
			sKerberos,
			sLast
		};

		AuthPhaseStat();
		AuthPhaseStat(const AuthPhaseStat &s);

		void reset();

		virtual OLog &store(OLog &log) const;
		virtual ILog &load(ILog &);
		bool sane() const;

		void all(TmSzStat &all) const;

		inline void record(const Scheme scheme, const Time &tm, const Size sz);
		inline const TmSzStat &get(const Scheme scheme) const;

		AuthPhaseStat &operator +=(const AuthPhaseStat &s);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		TmSzStat theStats[sLast];

		static const String SchemeNames[];
};

/* inlined methods */

inline
void AuthPhaseStat::record(const Scheme scheme, const Time &tm, const Size sz) {
	theStats[scheme].record(tm, sz);
}

inline
const TmSzStat &AuthPhaseStat::get(const Scheme scheme) const {
	return theStats[scheme];
}

#endif
