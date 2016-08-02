
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__KERBEROS_XACT_H
#define POLYGRAPH__KERBEROS_XACT_H

#include "xstd/FileScanner.h"
#include "xstd/Kerberos.h"
#include "xstd/NetAddr.h"
#include "xstd/Socket.h"
#include "runtime/IOBuf.h"

class Client;

namespace Kerberos {

class Mgr;

class Xact: public FileScanUser {
	public:
		Xact();
		virtual ~Xact();

		virtual const ::String &description() const = 0;
		virtual void reset();
		void exec(const NetAddr &aKdcAddr);

		bool needRetry() const { return doRetry; }
		//bool done() const { return theState == stDone; }
		int tryCount() const { return theTryCount; }

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);
		virtual void noteTimeout(int fd, Time tout);
		const NetAddr &kdcAddr() const { return theKdcAddr; }

		int sessionId; // Mgr tag to ignore late transactions

	protected:
		const Client &client() const;
		const Mgr &owner() const { Must(theOwner); return *theOwner; }
		Mgr &owner() { Must(theOwner); return *theOwner; }

		bool configure(Mgr &anOwner);
		virtual void finish(const bool isSuccessful = false);
		void finishAndRetry() { doRetry = true; finish(); }
		int logCat() const;
		void reportError(const char *action, const ErrorCode krbError = 0) const;
		void reportError(const ::Error polyErr, const char *action, const ErrorCode krbError = 0) const;
		virtual ErrorCode step(Data &in, Data &out) = 0;

		// Polygraph error used by kids when detailing lower level errors
		virtual ::Error polyError() const = 0;

		bool usingTcp; // whether we are or should be using a TCP connection

	private:
		bool openSocket();
		bool reOpenSocket(ErrorCode e);
		void closeSocket();
		void startIO(const IODir dir);
		void doStep();
		void reportErrorAndFinish(const char *action, const ::Error sysError);
		void reportErrorAndFinish(const ::Error polyError);
		void reportErrorAndFinish(const char *action, const ErrorCode krbError);
		void outAppend(IOBuf* &out ,const char* buf, Size sz);

	private:
		Mgr *theOwner;
		CCache *theCCache;
		NetAddr theKdcAddr;
		Socket theSock;
		FileScanReserv theReserv;
		IOBuf *theIn;
		IOBuf *theOut;
		IOBuf *theLastRequest;
		int theTryCount;
		bool doRetry;
		bool usingPconn; // sent a request on a previously used TCP connection
};

}; // namespace Kerberos

#endif
