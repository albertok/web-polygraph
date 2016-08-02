
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/BcastChannel.h"
#include "runtime/BcastRcver.h"

BcastRcver::~BcastRcver() {
	stopListen();
}

void BcastRcver::startListen() {
	for (int i = 0; i < theChannels.count(); ++i)
		theChannels[i]->subscribe(this);
}

void BcastRcver::stopListen() {
	for (int i = 0; i < theChannels.count(); ++i)
		theChannels[i]->unsubscribe(this);
}

void BcastRcver::noteAgentEvent(BcastChannel *, const Agent *) { Assert(false); }
void BcastRcver::noteClientEvent(BcastChannel *, const Client *) { Assert(false); }
void BcastRcver::noteConnEvent(BcastChannel *, const Connection *) { Assert(false); }
void BcastRcver::noteXactEvent(BcastChannel *, const Xaction *) { Assert(false); }
void BcastRcver::noteCompoundXactEvent(BcastChannel *, const CompoundXactInfo *) { Assert(false); }
void BcastRcver::noteIcpXactEvent(BcastChannel *, const IcpXaction *) { Assert(false); }
void BcastRcver::notePageEvent(BcastChannel *, const PageInfo *) { Assert(false); }
void BcastRcver::noteMsgStrEvent(BcastChannel *, const char *) { Assert(false); }
void BcastRcver::noteErrEvent(BcastChannel *, const Error &) { Assert(false); }
void BcastRcver::noteInfoEvent(BcastChannel *, InfoEvent) { Assert(false); }
void BcastRcver::noteLogEvent(BcastChannel *, OLog &) { Assert(false); }
