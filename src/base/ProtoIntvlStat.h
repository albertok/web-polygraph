
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_PROTOINTVLSTAT_H
#define POLYGRAPH__BASE_PROTOINTVLSTAT_H

#include "base/HRStat.h"
#include "base/LevelStat.h"

class ProtoProgress;

// interval statistics for a given protocol such as SSL/HTTPS or FTP
class ProtoIntvlStat {
	public:
		ProtoIntvlStat();

		void progress(ProtoProgress *aProgress);
		const char *id() const;
		const char *name() const;
		bool updateProgress;

		void restart();
		void keepLevel(const ProtoIntvlStat &s);

		bool active() const;
		bool sane() const;

		double reqRate(Time duration) const;
		double repRate(Time duration) const;
		double reqBwidth(Time duration) const;
		double repBwidth(Time duration) const;

		inline const HRStat &doneXacts() const;
		inline const LevelStat &xactLevel() const;
		inline const LevelStat &connLevel() const;
		inline int errXacts() const;

		void recordXact(const Time &tm, const Size &sz, const bool hit);
		inline LevelStat &xactLevel();
		inline LevelStat &connLevel();
		inline void recordXactError();

		void merge(const ProtoIntvlStat &s);
		void concat(const ProtoIntvlStat &s);

		OLog &store(OLog &log) const;
		ILog &load(ILog &log);

		ostream &print(ostream &os, const String &pfx, Time duration) const;
		void linePrint(ostream &os, int offset, Time duration, bool includeLevels) const;

	protected:
		void join(const ProtoIntvlStat &s);

	protected:
		ProtoProgress *theProgress; // global progress of this protocol
		HRStat theDoneXacts;    // successful PROTO transactions (aggr)
		LevelStat theXactLvl;   // successful PROTO transactions (level)
		LevelStat theConnLvl;   // open PROTO connections (level)
		int theErrXacts;        // not successful PROTO transactions (count)
};


inline OLog &operator <<(OLog &ol, const ProtoIntvlStat &s) { return s.store(ol); }
inline ILog &operator >>(ILog &il, ProtoIntvlStat &s) { return s.load(il); }

inline
const HRStat &ProtoIntvlStat::doneXacts() const {
	return theDoneXacts;
}

inline
const LevelStat &ProtoIntvlStat::xactLevel() const {
	return theXactLvl;
}

inline
const LevelStat &ProtoIntvlStat::connLevel() const {
	return theConnLvl;
}

inline
LevelStat &ProtoIntvlStat::xactLevel() {
	return theXactLvl;
}

inline
LevelStat &ProtoIntvlStat::connLevel() {
	return theConnLvl;
}

inline
void ProtoIntvlStat::recordXactError() {
	++theErrXacts;
}

inline
int ProtoIntvlStat::errXacts() const {
	return theErrXacts;
}

#endif
