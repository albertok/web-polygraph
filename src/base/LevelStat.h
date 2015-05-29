
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_LEVELSTAT_H
#define POLYGRAPH__BASE_LEVELSTAT_H

#include "xstd/Time.h"

class String;
class OLog;
class ILog;

// [concurrency] level statistics
class LevelStat {
	public:
		LevelStat();

		LevelStat &operator ++() { inc(); return *this; }
		LevelStat &operator --() { dec(); return *this; }

		bool active() const { return theIncCnt > 0 || theDecCnt > 0; }
		bool known() const { return theLevel != 0 || theStart >= 0; }
		int incCnt() const { return theIncCnt; }
		int decCnt() const { return theDecCnt; }
		int level() const { return theLevel; } 
		double mean() const;

		void restart();
		void store(OLog &log) const;
		void load(ILog &log);
		bool sane() const;

		void keepLevel(const LevelStat &prevLevel);
		void merge(const LevelStat &s);
		void concat(const LevelStat &s);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		void inc() { change(); theIncCnt++; theLevel++; }
		void dec() { change(); theDecCnt++; theLevel--; }
		void change();

		void join(const LevelStat &s);

	private:
		double nom() const;
		double denom() const;

	protected:
		int theIncCnt;
		int theDecCnt;
		int theLevel;     // internal, not reset on restart

		Time theStart;    // start of measurements
		Time theCurStart; // start of the current level
		Time theSum;      // "level-seconds" (nominator of an average)

	private:
		double theNom;    // these are used for joining
		double theDenom;  // LevelStats only
};


inline OLog &operator <<(OLog &log, const LevelStat &ls) { ls.store(log); return log; }
inline ILog &operator >>(ILog &log, LevelStat &ls) { ls.load(log); return log; }


#endif
