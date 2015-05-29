
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__SERVER_SRVXACT_H
#define POLYGRAPH__SERVER_SRVXACT_H

#include "runtime/Xaction.h"

class Server;

class SrvXact: public Xaction {
	public:
		SrvXact();

		virtual void reset();

		void exec(Server *anOwner, Connection *aConn, Time delay);

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);
		virtual void wakeUp(const Alarm &a);

	protected:
		virtual Agent *owner();
		virtual void doStart();
		virtual void finish(Error err);
		virtual void logStats(OLog &ol) const;
		virtual void noteBodyDataReady();
		virtual void noteBufReady();
		virtual void noteConnReady();
		virtual void noteDataReady();

		virtual void noteHdrDataReady() = 0;
		virtual void noteRepSent() = 0;
		virtual void makeRep(WrBuf &buf) = 0;

		void consume(Size);
		void overwriteUrl();
		bool grokUrl(const bool isHealthCheck);
		void grokForeignUrl(const String &url, bool &ignoreUrls);

	protected:
		Server *theOwner;
		Alarm theThinkTimeAlarm; // think time timeout alarm
};

#endif
