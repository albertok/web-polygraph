
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "xstd/Clock.h"
#include "base/ObjId.h"
#include "base/RndPermut.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "runtime/Agent.h"
#include "runtime/CompoundXactInfo.h"
#include "runtime/ErrorMgr.h"
#include "runtime/HostMap.h"
#include "runtime/ObjUniverse.h"
#include "runtime/SharedOpts.h"
#include "runtime/StatPhaseSync.h"
#include "runtime/PolyOLog.h"
#include "runtime/HttpPrinter.h"
#include "runtime/httpHdrs.h"
#include "runtime/httpText.h"
#include "runtime/Xaction.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/globals.h"
#include "csm/BodyIter.h"
#include "csm/ContentCfg.h"
#include "csm/oid2Url.h"

XactLiveQueue TheLiveXacts;
int Xaction::TheSampleDebt = 0;
Counter Xaction::TheCount = 0;


void Xaction::reset() {
	theConn = 0;
	theRepContentCfg = 0;
	theReqContentCfg = 0;
	theBodyIter = 0;
	theId.clear();
	theStartTime = theLifeTime = theContinueMsgTime = theLastReqByteTime = theFirstRespByteTime = Time();
	theRepSize.reset();
	theReqSize.reset();
	theAbortSize = -1;
	theAbortCoord.reset();
	theCheckAlg.reset();
	theHttpStatus = RepHdr::scUnknown;
	theOid.reset();
	theLogCat = lgcAll;
	theReqOid.reset();
	theReqFlags = -1;
	theRepFlags = -1;
	theError = Error();
	theState = stNone;
	protoStat = 0;
	theLifeTimeAlarm.clear();
}

void Xaction::start(Connection *aConn) {
	Assert(aConn);
	Should(!theConn || theConn == aConn);
	theConn = aConn;
	theConn->protoStat = protoStat;

	theId.create();

	theStartTime = theConn->useStart();

	TheLiveXacts.enqueue(this);

	Broadcast(TheXactBegChannel, this);
}

void Xaction::finish(Error err) {
	if (!theLiveXacts.isolated())
		TheLiveXacts.dequeue(this);
	else { // if lifetime timeout happens while in waiting queue
		Must(err);
		Must(err == errXactLifeTimeExpired);
	}

	if (theConn) {
		theConn->theWr.stop(this);
		theConn->theRd.stop(this);
	} else { // if lifetime timeout happens while in waiting queue
		Must(err);
		Must(err == errXactLifeTimeExpired);
	}

	cancelAlarms();

	theLifeTime = TheClock - theStartTime;

	// prefer theError over err
	if (theError)
		err = theError;

	if (err) {
		if (theConn)
			theConn->bad(true);
		if (needRetry()) {
			Broadcast(TheXactRetrChannel, this);
		} else {
			// low level errors should be reported when they occur
			// and propogated to us as "-1"; do not count/report them twice
			if (err != errOther) {
				if (ReportError(err) && theConn &&
					TheOpts.theDumpFlags(dumpErr, dumpAny))
					printMsg(theConn->theRdBuf);
			}
			countFailure();
		}
	} else {
		countSuccess();
		if (TheSampleDebt) {
			TheSampleDebt--;
			logStats();
		}
		if (TheOpts.theDumpFlags(dumpSum, dumpAny)) {
			printXactLogEntry();
		}
	}

	if (theBodyIter) {
		theBodyIter->putBack();
		theBodyIter = 0;
	}

	newState(stDone);
}

void Xaction::countSuccess() {
	Broadcast(TheXactEndChannel, this);
}

void Xaction::countFailure() {
	// note: may be called by an Agent before the transaction started
	// if, for example, launch failed
	Broadcast(TheXactErrChannel, this);
}

void Xaction::newState(State aState) {
	theState = aState;
}

// returns true iff the transaction should abort
bool Xaction::abortBeforeIo() const {
	return theAbortSize == 0;
}

// returns true iff the transaction should abort
bool Xaction::abortAfterIo(Size size) {
	if (!Should(theAbortSize != 0))
		return false; // internal error, but be robust

	if (theAbortSize < 0)
		return false; // abort is disabled

	if (size <= 0)
		return false; // xactions will handle errors and eof on their own

	if (theAbortSize <= size) {
		theAbortSize = 0;
		return true; // time to abort
	}

	theAbortSize -= size;
	return false; // not yet
}

// returns true iff finish() was called
bool Xaction::abortIo(Connection::IoMethod m, Size *size) {
	if (abortBeforeIo()) {
		abortNow();
		return true;
	}

	if (theAbortSize >= 0)
		theConn->decMaxIoSize(theAbortSize);
	const Size sz = (theConn->*m)();
	if (size)
		*size = sz;

	if (abortAfterIo(sz)) {
		abortNow();
		return true;
	}

	return false;
}

void Xaction::abortNow() {
	theConn->bad(true); // so that the manager will close
	theOid.aborted(true);
	finish(0); // abort is not an error, config asked for it
}

RndDistr *Xaction::seedOidDistr(RndDistr *raw, int globSeed) {
	if (raw) {
		const int seed = GlbPermut(theOid.hash(), globSeed);
		raw->rndGen()->seed(seed);
	}
	return raw;
}

void Xaction::logStats() {
	Assert(TheSmplOLog);
	(*TheSmplOLog) << bege(lgXactStats, logCat());
	logStats(*TheSmplOLog);
	(*TheSmplOLog) << ende;
}

void Xaction::logStats(OLog &ol) const {
	ol
		<< theId << theConn->seqId()
		<< theStartTime
		<< (int)queueTime().msec()
		<< (int)theLifeTime.msec()
		<< theOid
		<< theRepSize
		<< theHttpStatus
		;
}

void Xaction::printMsg(const IOBuf &buf) const {
	printMsg(buf, buf.contSize());
}

void Xaction::printMsg(const IOBuf &buf, Size maxSize) const {
	if (buf.contSize() <= 0)
		printMsg(0, 0);
	else
		printMsg(buf.content(), maxSize);
}

inline
bool myIsPrint(char c) {
	return (c && isprint(c)) || c == '\r' || c == '\n';
}

// print buf content up to the first non-printable character,
// and no more than maxSize characters
void Xaction::printMsg(const char *buf, Size maxSize) const {
	cout << TheClock << '#';

	if (theOid) {
		Oid2Url(theOid, cout << " obj: ");
		theOid.printFlags(cout << " flags: ");
	}

	cout << " size: " << (int)theRepSize.actual() << '/' << (int)theRepSize.expected();

	if (theId)
		cout << " xact: " << theId;

	if (theEnqueTime > 0)
		cout << " enque: " << theEnqueTime;

	if (theStartTime > 0)
		cout << " start: " << theStartTime;

	cout << endl;

	if (!buf) {
		cout << "[no data to dump]" << endl;
		return;
	}

	if (maxSize > TheOpts.theDumpSize)
		maxSize = TheOpts.theDumpSize;

	const char *p = buf;
	while ((p-buf) < maxSize && myIsPrint(*p)) ++p;

	if (p > buf) {
		cout.write(buf, p - buf);
		cout << endl;
	}
}

void Xaction::printXactLogEntry() const {
	// XXX: should dump to a file, not cout and use squid or common format+?
	cout << "xact: " << TheClock << ' ';
	cout << ' ' << theLifeTime.msec();
	cout << ' ' << ((Xaction*)this)->owner()->host(); // XXX: ugly cast
	cout << " -/" << theHttpStatus;
	cout << ' ' << (int)theRepSize.actual();
	cout << " -";

	if (theOid)
		Oid2Url(theOid, cout << ' ');
	else
		cout << " -";

	if (theCheckAlg.sum().set())
		theCheckAlg.sum().print(cout << ' ');
	else
		cout << " -";

	if (theRepSize.header().known())
		cout << ' ' << (int)theRepSize.header() << '/';
	else
		cout << " -/";
	if (theRepSize.actualBody().known())
		cout << (int)theRepSize.actualBody();
	else
		cout << '-';

	cout << endl;
}

// build and put checksum header field
void Xaction::putChecksum(ContentCfg &ccfg, const ObjId &oid, HttpPrinter &hp) const {
	if (!hp.putHeader(hfpContMd5))
		return;

	const xstd::Checksum sum(ccfg.calcChecksum(oid));
	PrintBase64(hp, sum.image(), sum.size()) << crlf;
}

void Xaction::updateUniverse(const ObjWorld &newWorld) {
	ObjUniverse &universe = *TheHostMap->findUniverseAt(theOid.viserv());
	universe.update(newWorld);
}

// report our readiness to change phase
void Xaction::putPhaseSyncPos(HttpPrinter &hp, const int pos) const {
	if (hp.putHeader(hfpXPhaseSyncPos))
		hp << pos << crlf;
}

// sync phases
void Xaction::doPhaseSync(const MsgHdr &hdr) const {
	if (hdr.thePhaseSyncPos >= 0 && hdr.theGroupId) {
		TheStatPhaseSync.notePhaseSync(hdr.theGroupId,
			hdr.thePhaseSyncPos);
	}
}

bool Xaction::preliminary() const {
	if (theOid.connect())
		return true;

	if (const CompoundXactInfo *info = partOf())
		return !info->completed();

	return false; // not a part of any compound transaction (yet?)
}

bool Xaction::authing() const { // XXX: review uses; final errors are not -ING!
	return httpStatus() == RepHdr::sc401_Unauthorized ||
		httpStatus() == RepHdr::sc403_Forbidden ||
		httpStatus() == RepHdr::sc407_ProxyAuthRequired ||
		conn()->theProxyNtlmState.state == ntlmError ||
		conn()->theOriginNtlmState.state == ntlmError;
}

// Cannot easily cache the result because the computed value may
// change, depending on xaction state.
bool Xaction::inCustomStatsScope() const {
	const Agent *const agent = const_cast<Xaction*>(this)->owner();
	Should(agent);
	const AgentCfg *const cfg = agent ? const_cast<Agent*>(agent)->cfg() : 0;
	return Should(cfg) && cfg->inCustomStatsScope(httpStatus());
}

void Xaction::lifeTimeLimit(const Time &lifetime) {
	Must(!theLifeTimeAlarm);
	if (lifetime >= 0)
		theLifeTimeAlarm = sleepFor(lifetime);
}

void Xaction::wakeUp(const Alarm &a) {
	AlarmUser::wakeUp(a);
	if (Should(theLifeTimeAlarm)) {
		theLifeTimeAlarm.clear();
		finish(errXactLifeTimeExpired);
	}
}
