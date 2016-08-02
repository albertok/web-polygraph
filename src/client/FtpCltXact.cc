
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/polyLogCats.h"
#include "base/StatIntvlRec.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/SharedOpts.h"
#include "client/Client.h"
#include "client/CltConnMgr.h"
#include "client/FtpCxm.h"
#include "client/FtpCltXact.h"
#include "client/ServerRep.h"
#include "csm/oid2Url.h"
#include "csm/BodyIter.h"
#include "csm/ContentCfg.h"
#include "runtime/globals.h"
#include "runtime/polyErrors.h"
#include "runtime/FtpText.h"

FtpCltXact::FtpCltXact() {
	FtpCltXact::reset();
}

void FtpCltXact::reset() {
	CltXact::reset();
	protoStat = &StatIntvlRec::theFtpStat;
	theReqOid.type(TheBodilessContentId);
	theRep.reset();
	theReqCmd = FtpReq::frcUnknown;
	theDataConn.reset();
	theDataConn.logCat(lgcCltSide);
	theDataConn.protoStat = protoStat;
	stopDataListen();
	stopDataChannel();
	theDataChannelState = dcsNone;
//Comment << here << "reset to new data state: " << theDataChannelState << endc;
}

void FtpCltXact::exec(Connection *const aConn) {
	CltXact::exec(aConn);
	newState(stHdrWaiting);

	Assert(!theMgr);
	theMgr = FtpCxm::Get();
	theMgr->control(this);
}

void FtpCltXact::finish(Error err) {
//Comment << here << "finishing " << err << endc;
	if (theDataChannelState == dcsNone && !err) // we have seen neither RETR nor error
		err = errFtpNoDataXfer;
	stopDataListen();
	stopDataChannel();
	stopCtrlChannel(err);
	CltXact::finish(err);
}

bool FtpCltXact::writeFirst() const {
	return false;
}

bool FtpCltXact::wantsToWrite() const {
	// we usually want to write after reading a reply, except
	// when we need to wait for the data connection to succeed
	return theDataChannelState != dcsConnecting;
}

PipelinedCxm *FtpCltXact::getPipeline() {
	return 0; // FTP cannot be a pipelined transaction
}

void FtpCltXact::pipeline(PipelinedCxm *) {
    // FTP does not support pipelining so we will not change theMgr
}

AuthPhaseStat::Scheme FtpCltXact::proxyStatAuth() const {
	return theOid.authCred() ? AuthPhaseStat::sFtp : AuthPhaseStat::sNone;
}

void FtpCltXact::stopCtrlChannel(const Error &err) {
	if (theMgr) {
		if (err)
			theMgr->noteAbort(this);
		else
			theMgr->noteDone(this);
		theMgr->release(this);
		theMgr = 0;
	}
}

bool FtpCltXact::controlledPostRead(bool &needMore) {
	Assert(theState == stHdrWaiting);

	if (Error err = handleReplies()) {
		finish(err);
		return false;
	}

	if (finishIfDone() || !theMgr)
		return false;

	needMore = theState == stHdrWaiting;
	return true;
}

bool FtpCltXact::finishIfDone() {
//Comment << here << "ctrl: " << theState << " data: " << theDataChannelState << endc;
//Comment << here << (theState == stDone) << " / " << (theDataChannelState == dcsDone) << " || " << (theDataChannelState == dcsNone) << endc;
	// do not quit if we are not done with the control channel
	if (theState != stDone) 
		return false;

	// quit if we are done with the data channel
	if (theDataChannelState == dcsDone || theDataChannelState == dcsNone) {
		finish(0);
		return true;
	}

	return false;
}

Error FtpCltXact::handleReplies() {
	IOBuf &buf = theConn->theRdBuf;
//Comment << "FTP reply (" << buf.contSize() << "): " << endc; printMsg(buf.content(), buf.contSize());

	// due to Preliminary and Intermediate replies, we might read several 
	// server replies at once; handle them one by one until we are done or err
	while (theRep.parse(buf.content(), buf.contSize())) {
//Comment << "FTP reply size: " << theRep.size() << endc;
//Comment << here << "ctrl: " << theState << " data: " << theDataChannelState << endc;
		theRepSize.expect(theRep.size()); // XXX: ignores data channel
		consume(theRep.size()); // XXX: does not work for data/ctrl conn split
		if (const Error err = interpretReply())
			return err;

//Comment << here << "ctrl: " << theState << " data: " << theDataChannelState << endc;

		if (theRep.code() >= 200) {
			if (!buf.empty()) { // leftovers?
				if (ReportError(errExtraRepData)) { // XXX: add and use errExtraReply
					Comment << "FTP reply leftovers (" << buf.contSize() <<
						")" << endc;
					if (TheOpts.theDumpFlags(dumpErr, dumpAny))
						printMsg(buf.content(), buf.contSize());
				}
				return errOther;
			}

			theReqSize.reset(); // XXX: why? should we do it after 150?
			if (theState == stHdrWaiting)
				newState(stSpaceWaiting); // signal that no more data is needed
			return 0;
		}

		theRep.reset();
		theRepSize.reset();
		// if we got a Positive Preliminary reply, check if is there more
	}

	if (buf.full()) // command too big
		return errFtpHugeCmd;

	if (theConn->atEof()) { // premature end of reply stream
		if (buf.empty())
			return errFtpPrematureEndOfCtrl; // expected a reply, got nothing
		else
			return errFtpPrematureEndOfMsg; // we did not get the entire reply
	}

	Assert(theState == stHdrWaiting);
	return 0; // need more data to parse the header
}

void FtpCltXact::makeReq(WrBuf &buf) {
	Assert(!theReqSize.expected().known());
	ofixedstream os(buf.space(), buf.spaceSize());
	switch (theReqCmd) {
		case FtpReq::frcUSER:
			makeUser(os);
			break;
		case FtpReq::frcPASS:
			makePass(os);
			break;
		case FtpReq::frcTYPE:
			makeType(os);
			break;
		case FtpReq::frcPASV:
			makePasv(os);
			break;
		case FtpReq::frcPORT:
			makePort(os);
			break;
		case FtpReq::frcRETR:
			makeRetr(os);
			break;
		case FtpReq::frcSTOR:
			makeStor(os);
			break;
		case FtpReq::frcQUIT:
			makeQuit(os);
			break;
		default:
			// if we get here, it is an intermal error, but we cannot finish()
			// TODO: need exceptions to properly handle this and other errors
			ReportError(errFtpCmdSequence);
			makeQuit(os);
	}
	theReqSize.expect(Size(os.tellp()));
	buf.appended(theReqSize.expected());
//Comment << "FTP request (" << buf.contSize() << "): " << endc; printMsg(buf.content(), buf.contSize());
}

Error FtpCltXact::interpretReply() {
	Assert(theState == stHdrWaiting);
	if (theRep.code() < 100 || theRep.code() >= 400)
		return errFtpCommandFailed;

	// Interpret the server response and make sure we are making progress 
	// according to client-side plan

	Error err = 0;
	// XXX: use named (enum) constants below
	// TODO: use a const transition table: rep.code -> expected state,
	//       next state?
	switch (theRep.code()) {
		case 220: // server greating
			err = transition(FtpReq::frcUnknown, FtpReq::frcUSER);
			if (!err && theSrvRep) {
				// we note request here to mimic HTTP; TODO: redesign?
				theSrvRep->noteRequest();
				if (true) // XXX: but there are FTP proxies; detect polysrv
					theSrvRep->noteFirstHandResponse();
			}
			break;
		case 331: // username accepted, password required
			err = transition(FtpReq::frcUSER, FtpReq::frcPASS);
			break;
		case 230: // password accepted.
			err = transition(FtpReq::frcPASS, FtpReq::frcTYPE);
			break;
		case 200:
			if (theReqCmd == FtpReq::frcTYPE) { // entering binary mode
				if (theOwner->usesPassiveFtp()) {
					err = transition(FtpReq::frcTYPE, FtpReq::frcPASV);
					theOid.passive(true);
				} else {
					err = transition(FtpReq::frcTYPE, FtpReq::frcPORT);
					theOid.active(true);
					if (!err)
						err = startDataListen();
				}
			} else // PORT accepted
				err = transition(FtpReq::frcPORT, theOid.put() ? FtpReq::frcSTOR : FtpReq::frcRETR);
			break;
		case 227: // passive address
			err = transition(FtpReq::frcPASV, theOid.put() ? FtpReq::frcSTOR : FtpReq::frcRETR);
			if (!err)
				err = interpretPasv();
			break;
		case 125: // Data connection already open; transfer starting.
		case 150: // Opening data connection.
			if (theReqCmd == FtpReq::frcRETR ||
				theReqCmd == FtpReq::frcSTOR) {
				if (theDataChannelState == dcsConnected)
					err = kickDataChannel();
				if (!err)
					err = transition(theReqCmd, FtpReq::frcUnknown);
			}
			break;
		case 226: // Operation completed
			if (theReqCmd != FtpReq::frcUnknown) { // optional(?) 150 not received
				err = transition(theReqCmd, FtpReq::frcQUIT);
				kickDataChannel();
			} else
				err = transition(FtpReq::frcUnknown, FtpReq::frcQUIT);
			break;
		case 221: // Bye.
			err = transition(FtpReq::frcQUIT, FtpReq::frcUnknown);
			theConn->lastUse(true); // no pconn support yet
			stopCtrlChannel(err);
			if (!err)
				newState(stDone); // XXX: misplaced, not the last call
			break;
		default:
			Assert(false); // XXX: report unknown status codes
	}

	return err;
}

Error FtpCltXact::transition(FtpReq::Command from, FtpReq::Command to) {
//Comment << here << "transition: " << theReqCmd << " =? " << from << " -> " << to << endc;
	if (theReqCmd != from)
		return errFtpCommandFailed; // XXX: report details and use errOther

	theReqCmd = to;
	return 0;		
}

// do not call Client::originAuthScheme: assume FTP always requires authentication
void FtpCltXact::makeUser(ostream &os) {
	os << ftpReqUserPfx;
	if (genCredentials()) { // TODO: call in start()
		os << theCred.name();
		Should(theOid.authCred()); // Client sets this
	} else {
		os << ftpReqUserAnonym;
		theOid.authCred(true); // anonymous but still authenticated
	}
	os << ftpReqUserSfx;
	theReqCmd = FtpReq::frcUSER;
}

void FtpCltXact::makePass(ostream &os) {
	os << ftpReqPassPfx;
    if (theCred.image())
		os << theCred.password();
	else
		os << ftpReqPassAnonym;
	os << ftpReqPassSfx;
	theReqCmd = FtpReq::frcPASS;
}

void FtpCltXact::makeType(ostream &os) {
	os << ftpReqTypeBinary;
	theReqCmd = FtpReq::frcTYPE;
}

void FtpCltXact::makePasv(ostream &os) {
	os << ftpReqPasv;
	theReqCmd = FtpReq::frcPASV;
}

void FtpCltXact::makePort(ostream &os) {
	Assert(theDataPort >= 0);
	const NetAddr addr(theOwner->host().addrN(), theDataPort);
	FtpMsg::PrintAddr(os << ftpReqPort, addr) << crlf;
	theDataPort = -1;
	theReqCmd = FtpReq::frcPORT;
}

void FtpCltXact::makeRetr(ostream &os) {
	Assert(theDataChannelState == dcsConnected || theDataChannelState == dcsListening);
	os << ftpReqRetr;
	Oid2UrlPath(theOid, os) << crlf;
	theReqCmd = FtpReq::frcRETR;
	if (theOid.foreignUrl())
		TheEmbedStats.foreignUrlRequested++;
}

void FtpCltXact::makeStor(ostream &os) {
	Assert(theDataChannelState == dcsConnected || theDataChannelState == dcsListening);
	Assert(!theReqContentCfg);
	Assert(!theBodyIter);

	os << ftpReqStor;
	Oid2UrlPath(theOid, os) << crlf;
	theReqCmd = FtpReq::frcSTOR;
	if (theOid.foreignUrl())
		TheEmbedStats.foreignUrlRequested++;

	theReqContentCfg = theOwner->selectReqContent(theOid, theReqOid);
	theBodyIter = theReqContentCfg->getBodyIter(theReqOid);
	theBodyIter->start(&theDataConn.theWrBuf);
}

void FtpCltXact::makeQuit(ostream &os) {
	os << ftpReqQuit;
	theReqCmd = FtpReq::frcQUIT;
}

Error FtpCltXact::interpretPasv() {
	const NetAddr addr(FtpMsg::ParseAddr(theRep.data()));
	return addr ? startDataChannel(addr) : errFtpBadPasv;
}

Error FtpCltXact::kickDataChannel() {
	if (theDataChannelState != dcsConnected)
		return errFtpCommandFailed;

	theDataChannelState = dcsInProgress;
//Comment << here << "starting to read data; new data state: " << theDataChannelState << endc;
	if (theReqCmd == FtpReq::frcRETR)
		theDataConn.theRd.start(this);
	else
		theDataConn.theWr.start(this);
	return 0;
}

Error FtpCltXact::startDataChannel(const NetAddr &addr) {
	if (theDataChannelState != dcsNone)
		return errFtpCmdSequence;
	// ConnMgr is used to get credentials for SOCKS proxy
	theDataConn.mgr(theOwner->connMgr());
	theDataConn.startUse();
	Should(theDataConn.connect(addr, SockOpt(), theOwner->portMgr(), theOwner->socksProxy()));
	theDataConn.theWr.start(this);
	theDataChannelState = dcsConnecting;
//Comment << here << "new data state: " << theDataChannelState << endc;
	return 0;
}

Error FtpCltXact::startDataListen() {
	if (theDataChannelState != dcsNone)
		return errFtpCmdSequence;

	Assert(theDataPort < 0);
	theSock = theOwner->makeListenSocket(theOwner->portMgr());
	if (!theSock) {
		if (ReportError2(Error::Last(), lgcCltSide))
			Comment << "fyi: FTP client could not create listening socket at " << theOwner->host() << endc;
		return errOther;
	}

	theDataPort = theSock.lport();
	Assert(theDataPort >= 0);

	theReserv = TheFileScanner->setFD(theSock.fd(), dirRead, this);
	theDataChannelState = dcsListening;

	return 0;
}

void FtpCltXact::stopDataChannel() {
	theDataConn.closeNow();
//Comment << here << "new data state: " << theDataChannelState << endc;
}

void FtpCltXact::stopDataListen() {
	if (theReserv)
		TheFileScanner->clearRes(theReserv);
	if (theSock)
		theSock.close();
	theDataPort = -1;
}

void FtpCltXact::noteReadReady(int fd) {
	if (fd == theSock.fd())
		acceptDataConnection();
	else
	if (fd == theDataConn.fd())
		noteDataChannelReadReady();
	else
		Assert(false);
}

void FtpCltXact::noteWriteReady(int fd) {
	if (fd == theDataConn.fd())
		noteDataChannelWriteReady();
	else
		Assert(false);
}

void FtpCltXact::acceptDataConnection() {
	Assert(theDataChannelState == dcsListening);
	Assert(!theDataConn.sock());
	theDataConn.startUse();
	theDataConn.protoStat = protoStat;
	bool fatal = false;
	Must(theDataConn.accept(theSock, SockOpt(), fatal));
//Comment << theSock.fd() << " accepted " << theDataConn.sock().fd() << endc;

	stopDataListen();
	theDataChannelState = dcsConnected;
	if (theReqCmd == FtpReq::frcRETR ||
		theReqCmd == FtpReq::frcSTOR)
		kickDataChannel();
}

void FtpCltXact::noteDataChannelReadReady() {
	Assert(theDataChannelState == dcsInProgress);

	// TODO: support simulated I/O aborts, MD5 checks
	const Size sz = theDataConn.read(); 
//Comment << "FTP data read: " << sz << " eof: " << theDataConn.atEof() << endc;
	if (theDataConn.bad()) {
		finish(errOther);
		return;
	}

	if (sz > 0) {
		theRepSize.got(sz);
		theDataConn.theRdBuf.consumed(sz);
		return; // only EOF signals the end of data
	}

	if (theDataConn.atEof()) {
		stopDataChannel();
		theDataChannelState = dcsDone;
//Comment << here << "new data state: " << theDataChannelState << endc;
		finishIfDone();
		return;
	}
}

void FtpCltXact::noteDataChannelWriteReady() {
//Comment << here << "ctrl: " << theState << " data: " << theDataChannelState << endc;
	switch (theDataChannelState) {
		case dcsConnecting:
			if (const Error err = theDataConn.sock().error()) {
//Comment << here << "connect err: " << err << endc;
				finish(err);
				return;
			}
			if (theDataConn.socksProxy() &&
				!theDataConn.socksConnected())
				theDataConn.socksWrite();
			else {
				theDataConn.theWr.stop(this);
				theDataChannelState = dcsConnected;
//Comment << here << "new data state: " << theDataChannelState << endc;
				// we can now send RETR/STOR
				// the response will kick reading on a data channel
				theMgr->resumeWriting(this);
			}
			break;
		case dcsInProgress:
			Assert(theBodyIter);
			theBodyIter->pour();
			theDataConn.write();
			if ((theBodyIter->pouredAll() &&
				theDataConn.theWrBuf.empty()) ||
				theDataConn.bad()) {
				stopDataChannel();
				theBodyIter->putBack();
				theBodyIter = 0;
				theDataChannelState = theDataConn.bad() ?
					dcsAborted : dcsDone;
			} else
			if (!theDataConn.theWr.theReserv)
				theDataConn.theWr.start(this);
			break;
		default:
			Assert(false);
	}
}

bool FtpCltXact::controlledFill(bool &needMore) {
	// do nothing while data channel is connecting
//	if (theDataChannelState == dcsConnecting)
//		return true;

	return CltXact::controlledFill(needMore);
}

bool FtpCltXact::controlledPostWrite(Size &size, bool &needMore) {
	// do nothing while data channel is connecting
//	if (theDataChannelState == dcsConnecting)
//		return true;

	return CltXact::controlledPostWrite(size, needMore);
}

bool FtpCltXact::controlledMasterRead() {
	// do nothing while data channel is reading
//	if (theDataChannelState == dcsInProgress)
//		return true;

	return CltXact::controlledMasterRead();
}
