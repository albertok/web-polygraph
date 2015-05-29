
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_POLYCLT_H
#define POLYGRAPH__CLIENT_POLYCLT_H

#include "app/PolyApp.h"

class LoadableModule;
class WssFreezer;

class PolyClt: public PolyApp {
	public:
		typedef Array<Client*> Clients;

	public:
		PolyClt();
		virtual ~PolyClt();

	protected:
		virtual void configure();
		virtual void reportCfg();

		virtual void startAgents();
		virtual void startServices();
		virtual void step();
		virtual void getOpts(Array<OptGrp*> &opts);
		virtual Agent *makeAgent(const AgentSym &agent, const NetAddr &address);
		virtual void logState(OLog &log);

		virtual const String sideName() const;
		virtual int logCat() const;

		virtual void noteInfoEvent(BcastChannel *ch, InfoEvent ev);
		virtual void noteClientEvent(BcastChannel *ch, const Client *c);

		virtual void loadPersistence();

		void loadModules(const Array<String*> &names);
		void startClients(int count);
		void stopClients(int count);
		Client *flipCltState(Clients &from, Clients &to);

	protected:
		Array<LoadableModule*> theModules;
		Clients thePopulus;   // active clients
		Clients theAvailClts; // other clients
		WssFreezer *TheWssFreezer;
};

#endif
