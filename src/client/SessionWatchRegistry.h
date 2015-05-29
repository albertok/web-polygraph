
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_SESSIONWATCHREGISTRY_H
#define POLYGRAPH__CLIENT_SESSIONWATCHREGISTRY_H

#include "runtime/ModuleRegistry.h"
#include "runtime/SessionWatch.h"
#include "runtime/BcastRcver.h"
#include "client/Client.h"

// registration handler for client-side data filters
class SessionWatchRegistry:  public ModuleRegistry< SessionWatch<Client> >,
	public BcastRcver {
	public:
		typedef void (Module::*Call)(const Client *c);

	public:
		SessionWatchRegistry();
		virtual ~SessionWatchRegistry();

		virtual void noteClientEvent(BcastChannel *ch, const Client *c);

	protected:
		void call(Call c, const Client *clt);
};

extern SessionWatchRegistry &TheSessionWatchRegistry();

LIB_INITIALIZER(SessionWatchRegistryInit)

#endif
