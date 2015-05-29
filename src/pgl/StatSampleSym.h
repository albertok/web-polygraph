
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_STATSAMPLESYM_H
#define POLYGRAPH__PGL_STATSAMPLESYM_H

#include "base/StatIntvlRec.h"
#include "pgl/PglExprSym.h"

// StatIntvlRec wrapper, usually created runtime by phase scripts
class StatSampleSym: public ExpressionSym {
	public:
		typedef StatIntvlRec Rec;

	public:
		static String TheType;

	public:
		StatSampleSym();
		StatSampleSym(const Rec &aRec);
		virtual ~StatSampleSym();

		virtual bool isA(const String &type) const;

		const Rec &value() const { return theRec; }

		virtual SynSymTblItem **memberItem(const String &name);

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		bool memberMatch(const String &prefix, const char *name, const char **tail) const;
		SynSym *memberLevel(const char *key, const LevelStat &stats) const;
		SynSym *memberHR(const char *key, const HRStat &stats, const String &nameHit, const String &nameMiss) const;
		SynSym *memberTmSz(const char *key, const TmSzStat &stats) const;
		SynSym *memberAggr(const char *key, const AggrStat &stats, const String &stype) const;
		SynSym *memberSym(double value, const String &stype) const;
		SynSym *memberProto(const char *key, const ProtoIntvlStat &stats) const;

	protected:
		Rec theRec;

	private:
		Array<SynSymTblItem*> theSymGarbage; // for garbage collection
};

#endif
