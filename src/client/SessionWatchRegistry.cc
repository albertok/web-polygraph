
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/polyBcastChannels.h"
#include "client/SessionWatchRegistry.h"


static SessionWatchRegistry *TheRegistry = 0;


SessionWatchRegistry::SessionWatchRegistry():
	ModuleRegistry<Module>("client-side session watches") {
	theChannels.append(TheSessionBegChannel);
	theChannels.append(TheSessionCntChannel);
	theChannels.append(TheSessionEndChannel);
	startListen();
}

SessionWatchRegistry::~SessionWatchRegistry() {
	stopListen();
}

void SessionWatchRegistry::noteClientEvent(BcastChannel *ch, const Client *c) {
        if (ch == TheSessionBegChannel) 
			call(&Module::noteStart, c);
		else
		if (ch == TheSessionCntChannel)
			call(&Module::noteHeartbeat, c);
		else
		if (ch == TheSessionEndChannel)
			call(&Module::noteEnd, c);
		else
			Should(false);
}

void SessionWatchRegistry::call(Call c, const Client *clt) {
	for (int i = 0; i < theModules.count(); ++i)
		(theModules[i]->*c)(clt);
}

SessionWatchRegistry &TheSessionWatchRegistry() {
	Assert(TheRegistry);
	return *TheRegistry;
}

/* initialization */

int SessionWatchRegistryInit::TheUseCount = 0;

void SessionWatchRegistryInit::init() {
	TheRegistry = new SessionWatchRegistry;
}

void SessionWatchRegistryInit::clean() {
	delete TheRegistry;
	TheRegistry = 0;
}
