
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/polyLogCats.h"
#include "client/Client.h"
#include "kerberos/Mgr.h"
#include "kerberos/Xact.h"
#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/globals.h"
#include "runtime/polyErrors.h"

namespace Kerberos {

// Whether all KDCs are believed to support persistent TCP connections.
// TODO: Make KDC-specific?
static bool UsePconnsWithKdc = true;

Xact::Xact() {
	theIn = new IOBuf();
	theOut = new IOBuf();
	theLastRequest = new IOBuf();
	reset();
}

Xact::~Xact() {
	closeSocket(); // we may be called w/o reset() when ObjFarm::put overflows
	delete theIn;
	delete theOut;
	delete theLastRequest;
}

void Xact::reset() {
	theOwner = 0;
	closeSocket();
	theIn->reset();
	theOut->reset();
	theLastRequest->reset();
	theTryCount = 0;
	sessionId = 0;
	doRetry = false;
	usingTcp = false;
	usingPconn = false;
}

bool Xact::configure(Mgr &anOwner) {
	Must(!theOwner);
	theOwner = &anOwner;
	usingTcp = anOwner.usingTcp;
	sessionId = anOwner.sessionId;
	return true;
}

void Xact::finish(const bool isSuccessful) {
	if (theSock)
		closeSocket();
	if (!doRetry)
		theLastRequest->reset();
	theOut->reset();
	theIn->reset();
}

void Xact::exec(const NetAddr &aKdcAddr) {
	++theTryCount;
	doRetry = false;

	theKdcAddr = aKdcAddr;
	Must(theKdcAddr);

	ShouldUs(!theSock);
	ShouldUs(theOut->empty());
	if (!theLastRequest->empty())
		theOut->append(theLastRequest->content(), theLastRequest->contSize());

	doStep();
}

void Xact::noteReadReady(int fd) {
	Must(fd == theSock.fd());
	Must(theIn->spaceSize() > 0); // to make sure our eof detection works
	const Size sz = theSock.read(theIn->space(), theIn->spaceSize());
	if (sz < 0) {
		const ::Error lastErr = ::Error::Last();
		if (lastErr != EWOULDBLOCK && lastErr != EAGAIN)
			reportErrorAndFinish("reading KDC response", lastErr);
		return;
	}

	if (sz == 0) {
		if (!usingTcp) {
			reportErrorAndFinish("reading KDC/UDP response", errKerberosKdcEof);
			return;
		}

		if (!usingPconn) {
			reportErrorAndFinish("reading KDC/TCP response", errKerberosKdcEof);
			return;
		}

		// We may be dealing with a KDC that does not support more than one
		// message per TCP connection. Remember that and reconnect the socket.
		UsePconnsWithKdc = false;
		closeSocket();
		theOut->reset();
		Must(theLastRequest->contSize() > 0); // we must have kept the request
		theOut->append(theLastRequest->content(), theLastRequest->contSize());
		doStep();
		return;
	}

	theIn->appended(sz);

	if (usingTcp) {
		/* extract message length prefix */
		uint32_t rawLength = 0;
		const int32_t pfxSize = sizeof(rawLength);
		if (theIn->contSize() < pfxSize)
			return;
		memcpy(&rawLength, theIn->content(), sizeof(rawLength));
		const uint32_t length = ntohl(rawLength);
		if (length > numeric_limits<int32_t>::max() - uint32_t(pfxSize)) {
			reportErrorAndFinish(errKerberosTcpPfx);
			return;
		}
		const int inSize = pfxSize + length;
		if (inSize > theIn->capacity()) {
			const int spaceSize = theIn->spaceSize();
			IOBuf *in = new IOBuf(inSize);
			in->append(theIn->content(), theIn->contSize());
			delete theIn;
			theIn = in;
			if (spaceSize == 0)
				noteReadReady(fd);
		}

		if (theIn->contSize() < static_cast<int32_t>(inSize))
			return; // did not get the entire message;

		theIn->consumed(pfxSize);
	}

	theOut->reset();
	theLastRequest->reset();

	if (usingTcp) {
		if (UsePconnsWithKdc)
			usingPconn = true;
		else
			closeSocket(); // will reconnect when needed
	}

	doStep();
}

void Xact::noteWriteReady(int fd) {
	Must(fd == theSock.fd());
	Must(!theOut->empty());

	const Size sz = theSock.write(theOut->content(), theOut->contSize());
	if (sz < 0) {
		reportErrorAndFinish("writing KDC request", ::Error::Last());
		return;
	}

	theOut->consumed(sz);
	if (theOut->empty())
		startIO(dirRead);
	else
	if (!usingTcp)
		finishAndRetry(); // UDP truncated our request
}

void Xact::noteTimeout(int fd, Time tout) {
	Must(fd == theSock.fd());
	reportErrorAndFinish(errKerberosKdcTimeout);
}

const Client &Xact::client() const {
	return owner().owner();
}

int Xact::logCat() const {
	return client().logCat();
}

bool Xact::openSocket() {
	const InAddress &addr = client().host().addrN();
	Must(addr.known());
	if (ShouldSys(theSock.create(addr.family(), (usingTcp ? SOCK_STREAM : SOCK_DGRAM), 0)) &&
		ShouldSys(theSock.blocking(false)) &&
		ShouldSys(theSock.reuseAddr(true)) &&
		ShouldSys(theSock.bind(NetAddr(addr, 0)))) {
		if (!theSock.connect(theKdcAddr)) {
			reportErrorAndFinish(usingTcp ? "connecting to KDC over TCP" :
				"connecting to KDC over UDP", ::Error::Last());
			return false;
		}
		return true;
	}

	reportError(usingTcp ? "opening TCP socket to KDC" : "opening UDP socket to KDC");
	return false;
}

bool Xact::reOpenSocket(ErrorCode e) {
	Must(!usingTcp);
	NetAddr *adrr = theOwner->switchToTcp();
	if (!adrr) {
		reportError("KDC-over-TCP server is needed but none are configured", e);
		finish(); // nothing is going to change if we retry
		return false;
	}

	closeSocket();
	theIn->reset();
	theOut->reset();
	theLastRequest->reset();

	theTryCount = 1;
	theKdcAddr = *adrr;
	usingTcp = true;

	return true;
}

void Xact::closeSocket() {
	if (theReserv)
		TheFileScanner->clearRes(theReserv);
	if (theSock)
		Should(theSock.close());
	usingPconn = false;
}

void Xact::startIO(const IODir dir) {
	if (theReserv)
		TheFileScanner->clearRes(theReserv);
	theReserv = TheFileScanner->setFD(theSock.fd(), dir, this);
	if (dir == dirRead)
		TheFileScanner->setTimeout(theSock.fd(), owner().timeout());
}

void Xact::reportError(const ::Error polyErr, const char *action, const ErrorCode krbErr) const {
	if (ReportError(polyErr)) {
		Comment << "error context: " << description() << " for " << owner().name();
		if (theKdcAddr)
			Comment << " from KDC " << theKdcAddr;

		if (action)
			Comment << endl << "failed action: " << action;
		if (krbErr)
			Comment << endl << "Kerberos details: " << Error(owner().context(), krbErr);

		if (tryCount())
			Comment << endl << "attemtps: " << tryCount();
		Comment << endc;
	}
}

void Xact::reportError(const char *action, const ErrorCode krbErr) const {
	reportError(polyError(), action, krbErr);
}

void Xact::doStep() {
	Data in(const_cast<char *>(theIn->content()), theIn->contSize());
	Data out;
	ErrorCode e = 0;
	if (!theOut->empty()) {
		// We get here after TCP pconn races and, via exec(), on retries.
		// We need to open a new socket and resend the same message.
	} else if ((e = step(in, out))) {
		if (e != KRB5KRB_ERR_RESPONSE_TOO_BIG || usingTcp) {
			finish(); // no retries on final Kerberos errors
			return;
		}
	}

	theIn->consumedAll();

	if (e == KRB5KRB_ERR_RESPONSE_TOO_BIG) {
		Comment(8) << "fyi: KDC response too big for UDP, retrying using TCP" << endc;
		if (!reOpenSocket(e))
			return;
	}

	if (!out.empty()) {
		const uint32_t rawLength = htonl(out.length());
		const uint32_t pfxSize = sizeof(rawLength);
		if (usingTcp)
			outAppend(theOut, reinterpret_cast<const char *> (&rawLength), pfxSize);
		outAppend(theOut, out.data(), out.length());
		// copy the last request message so that we can resend it
		outAppend(theLastRequest, theOut->content(), theOut->contSize());
	}

	if (theOut->empty())
		finish(true);
	else if (theSock || openSocket())
		startIO(dirWrite);
	else
		finishAndRetry(); // socket opening failure
}

void Xact::reportErrorAndFinish(const ::Error polyErr) {
	ReportError(polyErr);
	finishAndRetry();
}

void Xact::reportErrorAndFinish(const char *context, const ::Error sysErr) {
	reportError(sysErr, context);
	finishAndRetry();
}

void Xact::outAppend(IOBuf* &out ,const char* buf, Size sz) {
	if (out->spaceSize() < sz) {
		IOBuf *newOut = new IOBuf(sz + out->contSize());
		if (!out->empty())
			newOut->append(out->content(), out->contSize());
		delete out;
		out = newOut;
	}
	out->append(buf, sz);
}

}; // namespace Kerberos
