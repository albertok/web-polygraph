
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_NOTIFIER_H
#define POLYGRAPH__RUNTIME_NOTIFIER_H

#include "xstd/String.h"
#include "xstd/NetAddr.h"
#include "xstd/Socket.h"

class StatIntvlRec;

class Notifier {
	public:
		Notifier(const String &aLabel, const NetAddr &anAddr);
		~Notifier();

		bool active() const;

		void noteStats(const StatIntvlRec &s, int logCat);

	protected:
		String theLabel;
		NetAddr theAddr;
		Socket theSock;
};

#endif
