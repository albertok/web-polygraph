
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_SESSIONMGR_H
#define POLYGRAPH__CLIENT_SESSIONMGR_H

#include "xstd/AlarmClock.h"
#include "client/SessionStats.h"

class Client;
class CltCfg;
class CltXact;
class SessionHeartbitMgr;

class SessionMgr: public AlarmUser {
	public:
		SessionMgr(Client *anOwner);
		virtual ~SessionMgr();

		void configure(const CltCfg *aCfg);

		void start();
		void delayedStart(Time delay);
		void stop();

		void noteXactDone(CltXact *x);
		void noteXactErr(CltXact *x);

		virtual void wakeUp(const Alarm &a);
		virtual void bitHeart();

		bool idle() const { return isIdle; }

	protected:
		void stopHeartbit();

		void becomeBusy();
		void becomeIdle();
		void checkPoint();

	protected:
		Client *theOwner;
		const CltCfg *theCfg;
		SessionHeartbitMgr *theHeartbitMgr;
		SessionStats theStats;
		bool isIdle;
};

#endif
