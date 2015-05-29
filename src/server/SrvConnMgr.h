
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__SERVER_SRVCONNMGR_H
#define POLYGRAPH__SERVER_SRVCONNMGR_H

#include "runtime/ConnMgr.h"

class UniqId;

class SrvConnMgr: public ConnMgr {
	public:
		class User {
			public:
				virtual ~User() {}
				virtual void noteConnReady(Connection *conn) = 0;
				virtual const UniqId &id() const = 0;
				virtual bool writeFirst() const = 0;
		};

	public:
		SrvConnMgr(User *aUser);

		virtual void configureSsl(SslCtx *aCtx, const SslWrap *wrap);
		void acceptLmt(int aLmt);

		bool accept(Socket &s); // false iff fatal error

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);

		ProtoIntvlPtr protoStat; // passed-through to new connections

	protected:
		virtual void putIdle(Connection *conn);

	protected:
		User *theUser;
		int theAcceptLmt;
};

#endif
