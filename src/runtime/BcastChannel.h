
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_BCASTCHANNEL_H
#define POLYGRAPH__RUNTIME_BCASTCHANNEL_H

#include "xstd/Array.h"
#include "xstd/String.h"
#include "runtime/BcastRcver.h"

// event distribution channel

class BcastChannel: protected SchArray<BcastRcver*> {
	public:
		BcastChannel(const String &anEvName);

		int startBcast() const { if (Debug) showEvent(); return theCount; }
		BcastRcver* rcver(int idx) { return item(idx); }

		void subscribe(BcastRcver* rcver);
		void unsubscribe(BcastRcver* rcver);

	protected:
		void showEvent() const;

	public:
		static bool Debug; // debug flag is shared by all Channels

	protected:
		String theName;
};

template<class EvType>
inline
void Broadcast(BcastChannel *ch, EvType ev) {
	int i = ch->startBcast();
	while (--i >= 0)
		ch->rcver(i)->noteEvent(ch, ev);
}

#endif
