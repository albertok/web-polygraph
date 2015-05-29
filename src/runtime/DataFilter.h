
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_DATAFILTER_H
#define POLYGRAPH__RUNTIME_DATAFILTER_H

#include "xstd/String.h"

class IOBuf;

// common interface for various message headers and content filters
template <class Producer>
class DataFilter {
	public:
		virtual ~DataFilter() {}

		virtual String id() const = 0; // name-dash-version
		virtual void describe(ostream &os) const = 0; // a brief description

		// in-place filtering
		virtual void apply(Producer &p, IOBuf &buf) = 0;
};

#endif
