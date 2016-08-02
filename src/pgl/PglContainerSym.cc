
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/Rnd.h"
#include "base/StringArray.h"
#include "pgl/PglBoolSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglContainerSym.h"



String ContainerSym::TheType = "Container";



bool ContainerSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

bool ContainerSym::empty() const {
	return !count();
}

const SynSym *ContainerSym::item(int offset) const {
	double h;
	return itemProb(offset, h);
}

ExpressionSym *ContainerSym::bnOper(const Oper &op, const SynSym &exp) const {
	if (!op.equality())
		return ExpressionSym::bnOper(op, exp);

	if (!exp.isA(TheType))
		return ExpressionSym::bnOper(op, exp);

	const ContainerSym &them = (const ContainerSym&)exp.cast(TheType);

	const int thisCount = count();
	const int themCount = them.count();
	bool same = thisCount == themCount;

	for (int i = 0; same && i < thisCount; ++i) {
		const SynSym *s = item(i);
		if (s->isA(ExpressionSym::TheType)) {
			const ExpressionSym &thisE = 
				(const ExpressionSym &)s->cast(ExpressionSym::TheType);
			same = thisE.bnOper(Oper::Same(), *them.item(i));
		} else {
			// cannot compare non-expression items on equality,
			// but note that different counts will yeild a result (earlier)
			return ExpressionSym::bnOper(op, exp);
		}
	}

	if (op.same())
		return new BoolSym(same);
	if (op.diff())
		return new BoolSym(!same);

	return ExpressionSym::bnOper(op, exp);
}

void ContainerSym::forEach(Visitor &v, RndGen *const rng) const {
	const int c = count();
	for (int i = 0; i < c; ++i) {
		double p = -1;
		const SynSym *const s = itemProb(i, p);
		if (rng && probsSet() && p >= 0 && !rng->event(p))
				continue;

		if (s->isA(ContainerSym::TheType)) {
			const ContainerSym &c =
				(const ContainerSym&)s->cast(ContainerSym::TheType);
			c.forEach(v, rng);
		} else {
			v.visit(*s); // leaf
		}
	}
}
