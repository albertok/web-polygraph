
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLCONTAINERSYM_H
#define POLYGRAPH__PGL_PGLCONTAINERSYM_H

#include "pgl/PglExprSym.h"

class RndGen;
class StringArray;

// one expression can represent more than one SynSym,
// with optional per-item probabilities
class ContainerSym: public ExpressionSym {
	public:
		static String TheType;

		class Visitor {
			public:
				virtual ~Visitor() {}
				virtual void visit(const SynSym &item) = 0;
		};

	public:
		ContainerSym(const String &aType): ExpressionSym(aType) {}

		virtual bool empty() const;
		virtual int count() const = 0;
		virtual bool probsSet() const = 0;

		// offset must be between 0 and count-1
		virtual const SynSym *itemProb(int offset, double &prob) const = 0;

		const SynSym *operator[](int offset) const { return item(offset); }
		virtual const SynSym *item(int offset) const;
		//double prob(int offset) const;

		virtual bool isA(const String &type) const;

		virtual void forEach(Visitor &v, RndGen *const rng = 0) const = 0;

		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;
};

#endif
