
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_BCASTCHANNEL_H
#define POLYGRAPH__RUNTIME_BCASTCHANNEL_H

#include "xstd/Array.h"
#include "xstd/String.h"

class BcastRcver;

// event distribution channel

class BcastChannel: protected Array<BcastRcver*> {
	public:
		explicit BcastChannel(const String &anEvName, const bool willStopTrafficWaiting = false);

		int startBcast() const { if (Debug) showEvent(); return count(); }
		BcastRcver* rcver(int idx) { return item(idx); }

		void subscribe(BcastRcver* rcver);
		void unsubscribe(BcastRcver* rcver);

		bool stopsTrafficWaiting; // any received event ends the "wait" phase

	protected:
		void showEvent() const;

	public:
		static bool Debug; // debug flag is shared by all Channels

	protected:
		String theName;
};

#endif
