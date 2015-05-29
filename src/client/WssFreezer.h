
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_WSSFREEZER_H
#define POLYGRAPH__CLIENT_WSSFREEZER_H

#include "runtime/BcastRcver.h"

// base class for various objects that freeze working set size
class WssFreezer: public BcastRcver {
	public:
		WssFreezer();
		virtual ~WssFreezer() {}
		virtual void start();

	protected:
		void startFill();
		void startFreeze();

		virtual void printGoal(ostream &os) const = 0;

	private:
		static int TheStartCount;
};

#endif
