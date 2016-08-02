
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_XACTABORTCOORD_H
#define POLYGRAPH__RUNTIME_XACTABORTCOORD_H

#include "xstd/Rnd.h"
#include "xstd/Size.h"

// info required to compute whether and where to abort a transaction
class XactAbortCoord {
	public:
		XactAbortCoord();

		void configure(const RndGen::Seed whether, const RndGen::Seed where);
		void reset();

		operator void*() const { return
			theWhetherState && theWhereState ? (void*)-1 : 0; }

		// negative if no abort
		Size pos(Size hdrSize, Size bodySize) const;

		RndGen::Seed whether() const { return theWhetherState; }
		RndGen::Seed where() const { return theWhereState; }

	protected:
		RndGen::Seed theWhetherState;
		RndGen::Seed theWhereState;
};

#endif
