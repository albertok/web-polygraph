
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/StatIntvlRec.h"
#include "base/polyLogCats.h"
#include "runtime/LogComment.h"
#include "csm/BodyIter.h"
#include "csm/ContentMgr.h"
#include "runtime/globals.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "runtime/FtpText.h"
#include "runtime/SharedOpts.h"
#include "server/Server.h"

#include "server/FtpSrvXact.h"

FtpSrvXact::FtpSrvXact(): theDataPort(-1),
    theDataXferState(dxsNone), theDataIoState(disNone) {
	FtpSrvXact::reset();
}

void FtpSrvXact::reset() {
	SrvXact::reset();
	protoStat = &StatIntvlRec::theFtpStat;
	theReqOid.type(TheBodilessContentId);
	theReq.reset();
	theDir = "/";
	theFilename = String();
	stopDataListen();
	closeDataChannel();
	theDataXferState = dxsNone;
	theDataIoState = disNone;
	theDataConn.reset();
	theDataConn.logCat(lgcSrvSide);
	theWrittenSize.reset();
	thePortAddr = NetAddr();
	sent1xx = false;
}

void FtpSrvXact::finish(Error err) {
	stopDataListen();
	closeDataChannel();
	theRepSize = theWrittenSize; // XXX: ugly; what is reply?
	if (!theOid && !err) // we have seen neither RETR nor error
		err = errFtpNoDataXfer;
	SrvXact::finish(err);
}

void FtpSrvXact::noteReadReady(int fd) {
	if (fd == theSock.fd())
		acceptDataConnection();
	else
	if (fd == theDataConn.fd())
		noteDataChannelReadReady();
	else
		SrvXact::noteReadReady(fd);
}

void FtpSrvXact::noteWriteReady(int fd) {
	if (fd == theDataConn.fd())
		noteDataChannelWriteReady();
	else
		SrvXact::noteWriteReady(fd);
}

void FtpSrvXact::doStart() {
	newState(stHdrWaiting);
	theWrittenSize.expect(0); // will increase later
	SrvXact::doStart();
}

void FtpSrvXact::noteConnReady() {
	// Connection has just been established, send greeting
	newState(stSpaceWaiting);
	noteWriteReady(theConn->fd());
}

void FtpSrvXact::noteHdrDataReady() {
	IOBuf &buf = theConn->theRdBuf;
	if (theReq.parse(buf.content(), buf.contSize())) {
		theReqSize.expect(theReq.size());
//Comment << "FTP request (" << buf.contSize() << "): " << endc; printMsg(buf.content(), buf.contSize());
		if (const Error err = interpretCmd()) {
			finish(err);
			return;
		}
		// XXX: check for another command and quit, if any. Since we do not
		// and should not format replies before we can write them, we cannot
		// handle pipelined commands. The I/O code will not call us if there
		// are buffered leftovers but no new I/O activity.
		newState(stSpaceWaiting); // switch to writing ctrl response

		theConn->theRd.stop(this);
		consume(theReq.size());
		return;
	}

	if (theConn->theRdBuf.full()) { // command too big
		finish(errFtpHugeCmd);
		return;
	}

	if (theConn->atEof()) {
		if (!theConn->theRdBuf.empty())
			finish(errFtpPrematureEndOfMsg); // we did not get the entire cmd
		else
        if (theDataXferState == dxsInProgress ||
			theDataXferState == dxsConnWaiting)
			finish(errFtpPrematureEndOfCtrl); // control channel died first
		else
			finish(0); // assume the user decided not to send QUIT
		return;
	}

	// continue reading
}

void FtpSrvXact::noteRepSent() {
	theWrittenSize.got(theRepSize.actual());
	theRepSize.reset();

	if (theReq.cmd() == FtpReq::frcQUIT) {
		theConn->lastUse(true);
		finish(0);
		return;
	}

	// if not working on the data channel, expect more commands
	if (theDataXferState == dxsNone) {
		newState(stHdrWaiting);
		if (!theConn->theRd.theReserv) 
			theConn->theRd.start(this);
	} else
	if (!Should(theState == stSpaceWaiting)) // have not written final reply
		newState(stSpaceWaiting);

	// we may start sending data before we finish writing RETR response
	// hopefully this is OK (for passive?): packets may be reordered anyway
//	// after response is sent, start sending data if needed 
//	if (theDataXferState == dxsInProgress) {
//		Assert(theDataIoState == disBusy);
//		if (!theDataConn.theWr.theReserv)
//			theDataConn.theWr.start(this);
//	}
}

void FtpSrvXact::makeRep(WrBuf &buf) {
	Assert(!theRepSize.expected().known());
	ofixedstream os(buf.space(), buf.spaceSize());
	switch(theReq.cmd()) {
		case FtpReq::frcUnknown:
			makeSimpleResponse(os, ftpRspGreeting);
			break;
		case FtpReq::frcSYST:
			makeSimpleResponse(os, ftpRspSystem);
			break;
		case FtpReq::frcUSER:
			makeSimpleResponse(os, ftpRspUser);
			break;
		case FtpReq::frcPASS:
			makeSimpleResponse(os, ftpRspPass);
			break;
		case FtpReq::frcTYPE:
			makeSimpleResponse(os, ftpRspTypeBinary);
			break;
		case FtpReq::frcCWD:
			makeSimpleResponse(os, ftpRspCwd);
			break;
		case FtpReq::frcPWD:
			makeSimpleResponse(os, ftpRspPwdPfx + theDir + ftpRspPwdSfx);
			break;
		case FtpReq::frcMDTM:
			makeMdtmResponse(os);
			break;
		case FtpReq::frcSIZE:
			makeSizeResponse(os);
			break;
		case FtpReq::frcPASV:
			makePasvResponse(os);
			break;
		case FtpReq::frcPORT:
			makeSimpleResponse(os, ftpRspPort);
			break;
		case FtpReq::frcRETR:
		case FtpReq::frcSTOR:
			makeXferResponse(os);
			break;
		case FtpReq::frcQUIT:
			makeSimpleResponse(os, ftpRspQuit);
			break;
		case FtpReq::frcEPSV:
		case FtpReq::frcOther:
			makeSimpleResponse(os, ftpRspCmdNotImplemented);
			break;
		default:
			Assert(false);
	}
	theRepSize.expect(Size(os.tellp()));
	theWrittenSize.expectMore(theRepSize.expected());
	buf.appended(theRepSize.expected());
//Comment << "FTP reply (" << buf.contSize() << "): " << endc; printMsg(buf.content(), buf.contSize());
}

Error FtpSrvXact::interpretCmd() {
	Error err = 0;
	switch(theReq.cmd()) {
		case FtpReq::frcUSER:
		case FtpReq::frcPASS:
		case FtpReq::frcQUIT:
		case FtpReq::frcSYST:
		case FtpReq::frcPWD:
			break;
		case FtpReq::frcTYPE:
			if (!theReq.argument())
				err = errFtpNoArgument;
			else
			if (theReq.argument() != "I" &&
				theReq.argument() != "L 8")
				err = errFtpMode;
			break;
		case FtpReq::frcCWD:
			// TODO: trim theReq.argument() before use
			// TODO: Store "/" as static String?
			if (!theReq.argument())
				err = errFtpNoArgument;
			else
			if (theReq.argument().startsWith("/")) // absolute directory
				theDir = theReq.argument();
			else // relative directory
				theDir += theReq.argument(); 
			if (!theDir.endsWith("/", 1))
				theDir += "/";
			break;
		case FtpReq::frcMDTM:
		case FtpReq::frcSIZE:
			if (!theReq.argument())
				err = errFtpNoArgument;
			else
				theFilename = theReq.argument();
			break;
		case FtpReq::frcPASV:
			theOid.passive(true);
			err = startDataListen();
			break;
		case FtpReq::frcPORT:
			theOid.active(true);
			err = interpretPort();
			break;
		case FtpReq::frcRETR:
		case FtpReq::frcSTOR:
			err = processXferRequest();
			break;
		case FtpReq::frcEPSV:
			// we do not support it, but do not want to report it
			// TODO: add something like ReportErrorOnce(err, key)
			break;
		default:
			// report but continue
			if (ReportError(errFtpCommandNotImplemented) &&
				TheOpts.theDumpFlags(dumpErr, dumpAny))
                printMsg(theReq.image().data(), theReq.image().len());
	}
	return err;
}

Error FtpSrvXact::interpretPort() {
	thePortAddr = FtpMsg::ParseAddr(theReq.argument());
	return thePortAddr ? 0 : errFtpBadPort;
}

Error FtpSrvXact::startDataListen() {
	if (theDataIoState != disNone) {
		if (theDataXferState != dxsNone) // cannot reset connection now
			return errFtpCmdSequence;
		stopDataListen();
		closeDataChannel();
	}

	Assert(theDataIoState == disNone);
	Assert(theDataPort < 0);

	// TODO: support port manager so that we can use non-ephemeral ports
	const NetAddr addr(theOwner->host().addrN(), 0);
	theSock = theOwner->makeListenSocket(addr);
	if (!theSock) {
		if (ReportError2(Error::Last(), lgcSrvSide))
			Comment << "fyi: FTP server ran out of sockets or ephemeral ports at " << addr << endc;
		return errOther;
	}

	theDataPort = theSock.lport();
	Assert(theDataPort >= 0);

	theReserv = TheFileScanner->setFD(theSock.fd(), dirRead, this);
	theDataIoState = disListening;
//Comment << theSock.fd() << " listenting on " << addr << endc;
	return 0;
}

Error FtpSrvXact::startDataChannel() {
	if (theDataIoState != disNone) {
		if (theDataXferState != dxsNone) // cannot reset connection now
			return errFtpCmdSequence;
		stopDataListen();
		closeDataChannel();
	}

	Assert(theDataIoState == disNone);
	Assert(thePortAddr);

	theDataConn.startUse();
	Should(theDataConn.connect(thePortAddr, SockOpt(), theOwner->portMgr()));
	theDataConn.theWr.start(this);
	theDataIoState = disConnecting;
//Comment << here << "new data state: " << theDataChannelState << endc;
	return 0;
}

void FtpSrvXact::stopDataListen() {
	if (theDataIoState == disListening) {
		if (theReserv)
			TheFileScanner->clearRes(theReserv);
		if (theSock)
			theSock.close();
		theDataPort = -1;
		theDataIoState = disNone;
	}
}

void FtpSrvXact::closeDataChannel() {
	if (theDataIoState == disWait || theDataIoState == disBusy) {
		theDataConn.closeNow();
		theDataIoState = disNone;
	}
}

void FtpSrvXact::acceptDataConnection() {
	Assert(theDataIoState == disListening);
	Assert(!theDataConn.sock());
	theDataConn.startUse();
	theDataConn.protoStat = protoStat;
	bool fatal = false;
	Must(theDataConn.accept(theSock, SockOpt(), fatal));
//Comment << theSock.fd() << " accepted " << theDataConn.sock().fd() << endc;

	stopDataListen();
	theDataIoState = disWait; // but may start immediately below
	if (theDataXferState == dxsConnWaiting)
		startDataXfer();
}

void FtpSrvXact::noteDataChannelWriteReady() {
	switch (theDataIoState) {
		case disConnecting:
			if (const Error err = theDataConn.sock().error()) {
//Comment << here << "connect err: " << err << endc;
				finish(err);
				return;
			}

			theDataConn.theWr.stop(this);
			theDataIoState = disWait; // but may start immediately below
			if (theDataXferState == dxsConnWaiting)
				startDataXfer();
			break;
		case disBusy: {
			Assert(theDataXferState == dxsInProgress);
			Assert(theOid);
			Assert(theBodyIter);

			theBodyIter->pour();
			const Size sz = theDataConn.write();

			if (sz > 0)
				theWrittenSize.got(sz);

			if ((theBodyIter->pouredAll() && theDataConn.theWrBuf.empty()) ||
				theDataConn.bad()) {
				theDataXferState = theDataConn.bad() ? dxsAborted : dxsComplete;
				closeDataChannel();
				theBodyIter->putBack();
				theBodyIter = 0;

				// tell the client we are done writing data
				if (!theConn->theWr.theReserv)
					theConn->theWr.start(this);
				if (!Should(theState == stSpaceWaiting)) // internal error?
					newState(stSpaceWaiting);
			} else
			if (!theDataConn.theWr.theReserv)
				theDataConn.theWr.start(this);
			break;
		}
		default:
			Assert(false);
	}
}

void FtpSrvXact::noteDataChannelReadReady() {
	Assert(theDataIoState == disBusy);
	Assert(theDataXferState == dxsInProgress);

	const Size sz = theDataConn.read();

	if (sz > 0) {
		theReqSize.got(sz);
		theDataConn.theRdBuf.consumedAll();
	}

	if (sz == 0 || theDataConn.bad()) {
		theDataXferState = theDataConn.bad() ? dxsAborted : dxsComplete;
		closeDataChannel();

		// tell the client we are done reading data
		if (!theConn->theWr.theReserv)
			theConn->theWr.start(this);
		if (!Should(theState == stSpaceWaiting)) // internal error?
			newState(stSpaceWaiting);
	} else
	if (!theDataConn.theRd.theReserv)
		theDataConn.theRd.start(this);
}

Error FtpSrvXact::processXferRequest() {
	if (!theReq.argument())
		return errFtpNoArgument;
	theFilename = theReq.argument();

	if (theDataXferState != dxsNone)
		return errFtpCmdSequence;

	switch (theDataIoState) {
		case disNone:
			if (thePortAddr) {// got PORT
				// start connection
				theDataXferState = dxsConnWaiting;
				return startDataChannel();
			} else // expecting PASV or PORT before RETR/STOR
				return errFtpCmdSequence;

		case disListening: // got RETR before data connection
			// wait for the data connection; TODO: add timeout
			theDataXferState = dxsConnWaiting;
			return 0;

		case disWait: // got data connection before RETR/STOR
			return startDataXfer();

		case disConnecting: // internal error: early connection
		case disBusy: // internal error: early writing
			Should(false);
			return errOther;
	}

	Should(false); // internal error: unknown state
	return errOther;
}

// called when we are ready to start writing/reading data
Error FtpSrvXact::startDataXfer() {
	if (!Should(theDataXferState == dxsNone ||
		theDataXferState == dxsConnWaiting)) // internal inconsistency?
		return errOther;

	if (!Should(theDataConn.sock())) // internal inconsistency?
		return errOther;

	if (!Should(theDataIoState == disWait)) // internal inconsistency?
		return errOther;

	bool badUrl(false);
	if (theReq.cmd() == FtpReq::frcRETR) {
		if (prepareContent()) {
			theOid.get(true);
			theBodyIter->start(&theDataConn.theWrBuf);
			if (!theDataConn.theWr.theReserv)
				theDataConn.theWr.start(this);
		} else
			badUrl = true;
	} else {
		if (parseUrl()) {
			theReqSize.expect(Size()); // expected size unknown
			theOid.put(true);
			theReqOid.type(TheUnknownContentId);
			if (!theDataConn.theRd.theReserv)
				theDataConn.theRd.start(this);
		} else
			badUrl = true;
	}
	if (badUrl) {
		theDataXferState = dxsBadUrl;
		closeDataChannel(); // makeRetrResponse sends error
		return 0;
	}

	theDataXferState = dxsInProgress;
	theDataIoState = disBusy;

	return 0;
}

bool FtpSrvXact::prepareContent() {
	if (theBodyIter)
		return true;

	if (!parseUrl())
		return false;

	Assert(theOid.type() >= ContType::NormalContentStart());
	theRepContentCfg = TheContentMgr.get(theOid.type());
	theBodyIter = theRepContentCfg->getBodyIter(theOid);
	theWrittenSize.expectMore(theBodyIter->contentSize());
	return true;
}

void FtpSrvXact::makeSimpleResponse(ostream &os, const String &reply) {
	os << reply;
}

void FtpSrvXact::makeMdtmResponse(ostream &os) {
	// XXX: hard coded modification time
	// should be similar to IMS
	os << ftpRspMdtm << "20080714151541" << crlf;
}

void FtpSrvXact::makeSizeResponse(ostream &os) {
	if (prepareContent())
		os << ftpRspSize << (int)theBodyIter->contentSize() << crlf;
	else
		os << ftpRspBadUrl;

	// TODO: we should reset prepared content if RETR URL does not match
	// SIZE URL and vice versa
}

void FtpSrvXact::makePasvResponse(ostream &os) {
	Assert(theDataPort >= 0);
	const NetAddr addr(theOwner->host().addrN(), theDataPort);
	FtpMsg::PrintAddr(os << ftpRspPasvPfx, addr) << ftpRspPasvSfx;
	theDataPort = -1;
}

// Makes response for RETR/STOR
void FtpSrvXact::makeXferResponse(ostream &os) {
	switch (theDataXferState) {
		case dxsNone:
			Should(false); // internal error
			break;
		case dxsConnWaiting:
			os << ftpRspXferOpening;
			sent1xx = true;
			break;
		case dxsInProgress:
			os << ftpRspXferStarting;
			sent1xx = true;
			break;
		case dxsBadUrl:
			os << ftpRspBadUrl;
			theDataXferState = dxsNone;
			break;
		case dxsAborted:
			os << ftpRspXferAborted;
			theDataXferState = dxsNone;
			break;
		case dxsComplete:
			if (!sent1xx) { // many clients requrie 1xx responses
				os << ftpRspXferStarting;
				sent1xx = true;
			}
			os << ftpRspXferComplete;
			theDataXferState = dxsNone;
			break;
	}
}

bool FtpSrvXact::parseUrl() {
	const String url(theDir + theFilename);
	const char *p = url.data();
	if (!theOid.parse(p, p + url.len()))
		theOid.foreignUrl(url);

	grokUrl(false);

	if (theOid.foreignUrl()) {
		theOid.reset(); // will trigger errFtpNoDataXfer if needed
		return false;
	}

	return true;
}

/* notes
 *
 * PASV:
 * 		start listening, disListening
 *
 * RETR:
 * 		=dxsNone
 * 		!disBusy,
 * 		if disNone and PORT then start connection, disConnecting
 * 		if disListen then dxsConnWaiting
 * 		if disWait then dxsInProgress, write, disBusy
 *
 * accept:
 * 		=disListen
 * 		disWait
 * 		if dxsConnWaiting then dxsInProgress, write, disBusy
 */
