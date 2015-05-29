
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLQUALIFSYM_H
#define POLYGRAPH__PGL_PGLQUALIFSYM_H

#include "pgl/PglExprSym.h"

class QualifSym: public ExpressionSym {
	public:
		enum Kind { qfNone, qfLmt, qfNow, qfNmt };

	public:
		static const String TheType;

	public:
		QualifSym(Kind aKind = qfNone);

		virtual bool isA(const String &type) const;

		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		Kind kind() const { return theKind; }

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		Kind theKind;
};

#endif
