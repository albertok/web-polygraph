
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_SESSIONWATCH_H
#define POLYGRAPH__RUNTIME_SESSIONWATCH_H

#include "xstd/String.h"

// common interface for watching over Agent session changes
template <class Agent>
class SessionWatch {
	public:
		virtual ~SessionWatch() {}

		virtual String id() const = 0; // name-dash-version
		virtual void describe(ostream &os) const = 0; // a brief description

		virtual void noteStart(const Agent *a) = 0;    // called when session starts
		virtual void noteHeartbeat(const Agent *a) = 0; // session heartbeat
		virtual void noteEnd(const Agent *a) = 0;      // called when session ends
};

#endif
