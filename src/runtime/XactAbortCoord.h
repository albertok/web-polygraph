
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_XACTABORTCOORD_H
#define POLYGRAPH__RUNTIME_XACTABORTCOORD_H

#include "xstd/Size.h"

class RndGen;

// info required to compute whether and where to abort a transaction
class XactAbortCoord {
	public:
		XactAbortCoord();

		void configure(int whether, int where);
		void reset();

		operator void*() const { return
			theWhetherState && theWhereState ? (void*)-1 : 0; }

		// negative if no abort
		Size pos(Size hdrSize, Size bodySize) const;

		int whether() const { return theWhetherState; }
		int where() const { return theWhereState; }

	protected:
		int theWhetherState;
		int theWhereState;
};

#endif
