
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/polyLogCats.h"
#include "base/RndPermut.h"
#include "base/StatIntvlRec.h"
#include "csm/ContentMgr.h"
#include "runtime/polyErrors.h"
#include "runtime/LogComment.h"
#include "runtime/SharedOpts.h"
#include "server/Server.h"
#include "server/SrvCfg.h"
#include "server/SrvOpts.h"

#include "server/SrvXact.h"

SrvXact::SrvXact() {
	SrvXact::reset();
}

void SrvXact::reset() {
	Xaction::reset();
	theOwner = 0;
	theThinkTimeAlarm.clear();

	theLogCat = lgcSrvSide;
}

void SrvXact::exec(Server *anOwner, Connection *aConn, Time delay) {
	Assert(anOwner);
	theOwner = anOwner;
	theConn = aConn;

	const Time lifetime = theOwner->selectLifetime();
	lifeTimeLimit(lifetime);

	if (delay > 0) {
		if (lifetime < 0 || delay < lifetime)
			theThinkTimeAlarm = sleepFor(delay);
	} else
		doStart();
}

void SrvXact::noteReadReady(int) {
	if (abortIo(&Connection::read))
		return;

	if (theConn->bad())
		finish(errOther);
	else
		noteDataReady();
}

void SrvXact::noteWriteReady(int) {
	Assert(theState == stSpaceWaiting);
	WrBuf &buf = theConn->theWrBuf;

	if (!theRepSize.expected().known())
		makeRep(buf);

	// What is this? Is this needed?
	Assert(theRepSize.actual() > 0 || buf.contSize() > 0);

	noteBufReady();

	Size sz;
	if (abortIo(&Connection::write, &sz))
		return;

	if (theConn->bad()) {
		finish(errOther);
		return;
	}

	Assert(sz >= 0);
	if (sz > 0 && theFirstRespByteTime < 0)
		theFirstRespByteTime = TheClock - theStartTime;
	theRepSize.got(sz);
	if (theRepSize.gotAll()) {
		Should(!theRepSize.expectToGetLess());
		theConn->theWr.stop(this);
		noteRepSent();
	} else {
		Assert(theRepSize.expectToGetMore());
		if (!theConn->theWr.theReserv)
			theConn->theWr.start(this);
	}
}

void SrvXact::wakeUp(const Alarm &a) {
	if (theThinkTimeAlarm) {
		AlarmUser::wakeUp(a);
		theThinkTimeAlarm.clear();
		doStart();
	} else // lifetime expired
		Xaction::wakeUp(a);
}

Agent *SrvXact::owner() {
	return theOwner;
}

void SrvXact::doStart() {
	start(theConn);

	Assert(!theConn->theRd.theReserv); // cleared in conn mgr

	if (theConn->firstUse())
		noteConnReady();
	else
		// note: we assume that conn mgr read() on first noteReadReady()
		noteDataReady(); // push state machine forward
}

void SrvXact::finish(Error err) {
	Xaction::finish(err);
	theOwner->noteXactDone(this);
}

void SrvXact::logStats(OLog &ol) const {
	Xaction::logStats(ol);
	ol << theOwner->seqvId();
}

void SrvXact::noteBodyDataReady() {
	Must(false);
}

void SrvXact::noteBufReady() {
}

void SrvXact::noteConnReady() {
	Must(false);
}

void SrvXact::noteDataReady() {
	Assert(theState == stHdrWaiting || theState == stBodyWaiting);
	if (theState == stHdrWaiting)
		noteHdrDataReady();
	if (theState == stBodyWaiting)
		noteBodyDataReady();

	switch (theState) {
		case stHdrWaiting:
		case stBodyWaiting:
			if (!theConn->theRd.theReserv)  // need to read more
				theConn->theRd.start(this);
			break;
		case stSpaceWaiting:
			if (!theConn->theWr.theReserv)  // need to write more
				theConn->theWr.start(this);
			break;
		case stDone:
		case stNone:
			break;
		default:
			Assert(false);
	}
}

void SrvXact::consume(Size size) {
	theConn->theRdBuf.consumed(size);
	theReqSize.got(size);
}

void SrvXact::overwriteUrl() {
	// XXX: theOid.visName() will not be set!
	// we have to ignore host settings;
	// should we create our own worlds??
	static RndGen rng;
	theOid.world().create();
	theOid.target(theOwner->hostIdx());
	theOid.type(ContType::NormalContentStart());
	theOid.name(rng.trial32() | 1);
	theOid.foreignSrc(false);
	theOid.foreignUrl(0);
}

bool SrvXact::grokUrl(const bool isHealthCheck) {
	// XXX: pxy server cannot support ignoreUrls; see Server::hostIdx
	bool ignoreUrls =
		isHealthCheck ||
		(TheSrvOpts.ignoreUrls && theOwner->hostIdx() >= 0);

	if (ignoreUrls)
		overwriteUrl();
	else
	if (const String &url = theOid.foreignUrl())
		grokForeignUrl(url, ignoreUrls);

	return ignoreUrls;
}

void SrvXact::grokForeignUrl(const String &url, bool &ignoreUrls) {
	TheEmbedStats.foreignUrlRequested++;

	if (url.cmp("/pg/embed/", 10) == 0) {
		const int commaPos = url.find(',');

		// use url trailer (as is, base64 encoded, 24 bytes?) for seeds
		if (commaPos != String::npos) {
			const int seedSize = 3*sizeof(int);
			const Area &worldStr = url.area(commaPos+1, seedSize);
			const Area &nameStr = url.area(commaPos+1 + seedSize, seedSize);

			theOid.world() = worldStr.size() >= seedSize ?
				UniqId::FromStr(worldStr) : UniqId::Create();

			static RndGen rng;
			if (nameStr.size() >= seedSize) {
				int seed[4] = { 0, 0, 0, 0 };
				memcpy(&seed, nameStr.data(), Min((int)sizeof(seed), nameStr.size()));
				rng.seed(LclPermut(seed[0] + seed[2], seed[1] + seed[3]));
			}
			theOid.name(rng.trial32() | 1);
		}

		const Area &category = url.area(10, commaPos - 10);
		if (theOwner->cfg()->setEmbedContType(theOid, category)) {
			theOid.target(theOwner->hostIdx());
			theOid.foreignSrc(false);
			theOid.foreignUrl(String());
		} else {
			static int count = 0;
			if (++count <= 100) {
				Comment(5) << "error: bad content type in URL: " << endc;
				printMsg(theConn->theRdBuf, theReqSize.expected());
			}
			ignoreUrls = true;
			overwriteUrl(); // or we would not accept it
		}

//		TheEmbedStats.scriptSeen++;
	} else
	if (theOwner->cfg()->findRamFile(url, theOid)) {
		theOid.world().create();
		theOid.target(theOwner->hostIdx());
		theOid.foreignSrc(false);
		theOid.foreignUrl(String());
		ignoreUrls = true;
	} else
	if (TheOpts.acceptForeignMsgs) {
		static bool informed = false;
		if (!informed) {
			Comment(5) << "fyi: received first foreign request URL: " << endc;
			printMsg(theConn->theRdBuf, theReqSize.expected());
			informed = true;
		}
		ignoreUrls = true;
		overwriteUrl(); // or we would not accept it
	}
}
