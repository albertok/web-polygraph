
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_CONNECTION_H
#define POLYGRAPH__RUNTIME_CONNECTION_H

#include "xstd/Queue.h"
#include "xstd/NetAddr.h"
#include "xstd/Socket.h"
#include "xstd/FileScanner.h"
#include "base/ConnCloseStat.h"
#include "base/ProtoStatPtr.h"
#include "runtime/IOBuf.h"
#include "runtime/NtlmAuthState.h"

class ConnMgr;
class PortMgr;
class Socks;
class SslCtx;
class SslSession;
class Ssl;
class UserCred;

// bi-directional connection based on a socket with I/O buffers

class Connection {
	public:
		typedef ConnCloseStat::CloseKind CloseKind;
		typedef Size (Connection::*IoMethod)();

		class HalfPipe {
			public:
				HalfPipe(Connection &aConn, IOBuf &aBuf, IODir aDir):
					theConn(aConn), theBuf(aBuf), theDir(aDir) { reset(); }

				void reset();
				void start(FileScanUser *u);
				void start(FileScanUser *u, Time timeout);
				void stop(FileScanUser *u);
				void changeUser(FileScanUser *uOld, FileScanUser *uNew);

			public:
				Connection &theConn;
				IOBuf &theBuf;
				IODir theDir;
				FileScanReserv theReserv;
				int theIOCnt;
				bool isReady;
		};
		friend class HalfPipe;

		class NtlmAuth {
			public:
				NtlmAuth(): state(ntlmNone), useSpnegoNtlm(false) {}

				NtlmAuthState state;
				bool useSpnegoNtlm;
				String hdrRcvdT2;
		};

	public:
		Connection();
		~Connection() { if (theSock) closeNow(); }

		void reset();

		int seqId() const { return theSeqId; }
		Socket &sock() { return theSock; }
		int fd() const { return theSock.fd(); }
		const NetAddr &raddr() const { return theAddr; }
		NetAddr laddr() const;
		int lport() const { return theLocPort; }
		int rport() const;
		ConnMgr *mgr() const { return theMgr; }
		int logCat() const { return theLogCat; }

		bool exhausted() const { return isAtEof && !theRdBuf.contSize(); }
		bool atEof() const { return isAtEof; }
		bool bad() const { return isBad; }
		bool firstUse() const { return isFirstUse; }
		CloseKind closeKind() const { return theCloseKind; }

		const NetAddr &socksProxy() const { return theSocksProxy; }
		bool socksConnected() const { return theSocksProxy && !theSocks; }
		bool socksAuthed() const;
		Size socksWrite();

		bool sslConfigured() const { return theSslCtx != 0; }
		const Ssl *sslActive() const { return theSsl; }
		bool sslActivate();
		SslSession *sslSession();
		void sslSessionForget();

		bool connect(const NetAddr &addr, const SockOpt &opt, PortMgr *aPortMgr, const NetAddr &socksProxy = NetAddr());
		bool accept(Socket &s, const SockOpt &opt, bool &fatal);
		Size read();
		Size write();

		bool closing() const { return theCloseKind != ConnCloseStat::ckNone; }
		bool closeNow(); // closes everything
		// initiates SSL close, returns true if done; does not close raw sock
		bool closeAsync(FileScanUser *u, bool &fatal); 

		void mgr(ConnMgr *aMgr) { theMgr = aMgr; }
		void logCat(int aLogCat) { theLogCat = aLogCat; }
		void useSsl(const SslCtx *aCtx, SslSession *aSess);

		bool pipelineable() const;
		bool reusable() const;
		void useCountLimit(int aLimit) { theUseCountLmt = aLimit; }
		void useLevelLimit(int aLimit) { theUseLevelLmt = aLimit; }
		int useCnt() const { return theUseCnt; }
		int useLevel() const { return theUseLvl; }
		void startUse();
		void finishUse();
		bool inUse() const { return theUseLvl > 0; }
		Time useStart() const { return theUseStart; }

		bool isIdle() const { return theUseLvl == 0; }
		int useLevelMax() const { return theUseLvlMax; }

		bool tunneling() const { return theTunnel.known() && theTunnel != raddr(); }
		const NetAddr &tunnelEnd() const { return theTunnel; }
		void tunnelEnd(const NetAddr &addr) { theTunnel = addr; }

		Time openTime() const { return theOpenTime; }
		int ioCnt() const { return theRd.theIOCnt + theWr.theIOCnt; }

		void bad(bool be) { isBad = be; }
		void lastUse(bool be) { isLastUse = be; }
		void closeKind(CloseKind aKind) { theCloseKind = aKind; }

		void decMaxIoSize(Size aMax);

		bool hasCredentials() const;
		bool genCredentials(UserCred &cred) const;

		void reportErrorLoc() const;

	public:
		QueuePlace<Connection> idleConnections;

	protected:
		void sslStart(int role);
		bool sslAccept();
		bool sslConnect();
		Size sslRead(Size ioSz);
		Size sslWrite(Size ioSz);
		bool sslCloseNow();
		bool sslCloseAsync(FileScanUser *u, bool &fatal);
		void sslForget();
		void sslError(int err, const char *operation);

		void socksStart();
		void socksEnd();
		Size socksRead();
		enum SocksOp { sopRead, sopWrite };
		Size socksOp(const SocksOp op);

		Size rawRead(Size ioSz);
		Size rawWrite(Size ioSz);
		bool rawCloseNow();
		void rawError(const char *operation);

		bool setSockOpt(const SockOpt &opt);
		bool preIo(HalfPipe &ioPipe, const char *operation);

	public:
		ProtoIntvlPtr protoStat; // protocol-specific stats for this conn

		RdBuf theRdBuf;
		WrBuf theWrBuf;

		HalfPipe theRd; // these refer to bufs; must go after them!
		HalfPipe theWr;

		NtlmAuth theProxyNtlmState; // proxy NTLM authentication state
		NtlmAuth theOriginNtlmState; // origin NTLM authentication state

	protected:
		Socket theSock;
		NetAddr theAddr;
		mutable ConnMgr *theMgr;
		PortMgr *thePortMgr;     // used to pass address to bind call
		const SslCtx *theSslCtx; // set by owner to request ssl encription
		SslSession *theSslSession; // set by owner if resumption is needed

		NetAddr theSocksProxy;  // the SOCKS proxy address if used
		NetAddr theTunnel;      // the other end of the tunnel if tunneling
		int theLocPort;
		mutable int theRemPort; // used as a cache for sock().rport()

		Time theOpenTime;
		Time theUseStart;       // start of the current/last "use"
		int theUseCountLmt;     // maximum total number of uses allowed
		int theUseLevelLmt;     // maximum total number of uses allowed
		int theUseCnt;          // number of "uses" since open()
		int theUseLvl;          // number of current concurrent "uses"
		int theUseLvlMax;       // maximum use level since open()
		CloseKind theCloseKind; // how the connection was closed

		Size theMaxIoSize;      // per-io limit, reset after each IO

		int theLogCat;          // log entry category

		bool isBad;
		bool isAtEof;
		bool isLastUse;         // this must be the last use of the conn
		bool isFirstUse;	// connection has just been established
		bool isSocksAuthed;	// authenticated with SOCKS proxy
		bool wasAnnounced;	// true if connection open event was sent

	private:
		static int TheLastSeqId;
		int theSeqId;

		// created internally if needed,
		// used only during initial connection negotiation
		Socks *theSocks;

		Ssl *theSsl;            // created internally if needed
};

#endif
