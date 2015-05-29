
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLBWIDTHSYM_H
#define POLYGRAPH__PGL_PGLBWIDTHSYM_H

#include "xstd/Time.h"
#include "xstd/BigSize.h"
#include "pgl/PglExprSym.h"

class TimeSym;
class SizeSym;
class NumSym;

typedef double Bwidth;  // for now
extern Bwidth Bandwidth(BigSize sz, Time tm);
inline double BytesPerSec(Bwidth bw) { return bw; }
inline double BitsPerSec(Bwidth bw) { return 8*BytesPerSec(bw); }


// bandwidth [size/time]

class BwidthSym: public ExpressionSym {
	public:
		static String TheType;

	public:
		BwidthSym(BigSize aSize, Time anInterval);
		BwidthSym(Bwidth aVal);

		Bwidth val() const;
		void val(Bwidth aVal);

		virtual bool isA(const String &type) const;

		virtual ExpressionSym *unOper(const Oper &op) const;
		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		ExpressionSym *operBB(const Oper &op, BwidthSym *rs, const SynSym &s) const;
		ExpressionSym *operBT(const Oper &op, TimeSym *ts, const SynSym &s) const;
		ExpressionSym *operBN(const Oper &op, NumSym *ns, const SynSym &s) const;

	protected:
		Bwidth theVal;
};




#endif
