
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_XACTION_H
#define POLYGRAPH__RUNTIME_XACTION_H

#include "xstd/AlarmClock.h"
#include "xstd/Error.h"
#include "xstd/FileScanner.h"
#include "xstd/Checksum.h"
#include "xstd/Queue.h"
#include "base/ObjId.h"
#include "base/ProtoStatPtr.h"
#include "runtime/httpHdrs.h"
#include "runtime/MessageSize.h"
#include "runtime/XactAbortCoord.h"
#include "runtime/Connection.h"

class IOBuf;
class Agent;
class RndDistr;
class ContentCfg;
class BodyIter;
class CompoundXactInfo;
class HttpPrinter;

class Xaction: public AlarmUser, public FileScanUser {
	public:
		static int TheSampleDebt; // log xactions until no debt

	public:
		virtual void reset(); // XXX: split into local/virtual reset

		const UniqId &id() const { return theId; }
		const ObjId &oid() const { return theOid; }
		const ObjId &reqOid() const { return theReqOid; }
		Connection *conn() { return theConn; }
		const Connection *conn() const { return theConn; }
		int logCat() const { return theLogCat; }
		bool started() const { return theStartTime > 0; }
		bool finished() const { return theState == stDone; }
		bool socksConfigured() const { return theConn && theConn->socksProxy(); }
		bool usedSocks() const { return theConn && theConn->usedSocks; }
		bool sslConfigured() const { return theConn && theConn->sslConfigured(); }
		bool usedSsl() const { return theConn && theConn->usedSsl; }

		void countSuccess();
		void countFailure();

		const MessageSize &repSize() const { return theRepSize; }
		const MessageSize &reqSize() const { return theReqSize; }
		const Time &startTime() const { return theStartTime; }
		Time queueTime() const { return theEnqueTime > 0 ? theStartTime - theEnqueTime : Time::Sec(0); }
		Time lifeTime() const { return theLifeTime; }
		Time continueMsgTime() const { return theContinueMsgTime; }
		Time lastReqByteTime() const { return theLastReqByteTime; }
		Time firstRespByteTime() const { return theFirstRespByteTime; }
		int httpStatus() const { return theHttpStatus; }
		bool authing() const;
		bool inCustomStatsScope() const;
		// This "helper" transaction is only meant to establish a tunnel and/or
		// authenticate the user before the expected "primary" transaction.
		bool preliminary() const;

		virtual bool needRetry() const { return false; }
		virtual void wakeUp(const Alarm &a);

		void oid(const ObjId &anOid) { theOid = anOid; }

		enum Flag {
			xfValidation = 1
		};

		int reqFlags() const { return theReqFlags; }
		int repFlags() const { return theRepFlags; }

		void lifeTimeLimit(const Time &lifetime);

		virtual int actualRepType() const { return theOid.type(); }
		virtual AuthPhaseStat::Scheme proxyStatAuth() const { Must(false); return AuthPhaseStat::sNone; }
		virtual bool startedXactSequence() const { return true; }
		virtual const CompoundXactInfo *partOf() const { return 0; }

		virtual int cookiesSent() const { return -1; }
		virtual int cookiesRecv() const { return -1; }

		ProtoIntvlPtr protoStat; // protocol-specific stats

		QueuePlace<Xaction> theLiveXacts;

	protected:
		enum State { stNone = 0, stConnWaiting,	stSpaceWaiting,
			stHdrWaiting, stBodyWaiting, stDone };

	protected:
		virtual Agent *owner() = 0;
		virtual void newState(State aState);

		void start(Connection *conn);
		virtual void finish(Error err);

		bool abortBeforeIo() const;
		bool abortAfterIo(Size size);
		bool abortIo(Connection::IoMethod m, Size *size = 0);
		virtual void abortNow();

		RndDistr *seedOidDistr(RndDistr *raw, int globSeed);

		void logStats();
		virtual void logStats(OLog &ol) const;

		void printMsg(const IOBuf &buf) const;
		void printMsg(const IOBuf &buf, Size maxSize) const;
		void printMsg(const char *buf, Size maxSize) const;
		void printXactLogEntry() const;

		void putChecksum(ContentCfg &ccfg, const ObjId &oid, HttpPrinter &hp) const;
		void updateUniverse(const ObjWorld &newWorld);

		void putPhaseSyncPos(HttpPrinter &hp, const int pos) const;
		virtual void doPhaseSync(const MsgHdr &hdr) const;

	protected:
		static Counter TheCount; // to report xaction "position"

	protected:
		Connection *theConn;
		ContentCfg *theRepContentCfg; // content config of reply body
		ContentCfg *theReqContentCfg; // content config of request body
		BodyIter *theBodyIter; // body iterator for request or response body
		UniqId theId;

		Time theEnqueTime;  // put in a queue (optional)
		Time theStartTime;  // started doing something
		Time theLifeTime;   // [start, stop]; set in stop()
		Time theContinueMsgTime; // 100-Continue received/sent, XXX: not reported
		Time theLastReqByteTime; // [start, last request byte]
		Time theFirstRespByteTime; // [start, first response byte]
		ObjId theOid;       // various details about the transfered object
		MessageSize theRepSize; // reply size
		MessageSize theReqSize; // request size
		Size theAbortSize;  // decremented on each I/O; abort if zero
		XactAbortCoord theAbortCoord; // other side abort coordinates
		xstd::ChecksumAlg theCheckAlg; // not used by default
		int theHttpStatus;
		int theLogCat;      // log entry category

		ObjId theReqOid;    // request content details; client-side only, for now

		int theReqFlags;
		int theRepFlags;

		Error theError;
		State theState;

		Alarm theLifeTimeAlarm; // life time timeout alarm
};

typedef Queue<Xaction, &Xaction::theLiveXacts> XactLiveQueue;
extern XactLiveQueue TheLiveXacts;

#endif
