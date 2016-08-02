
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "xstd/Ssl.h"
#include "base/polyLogCats.h"
#include "runtime/PortMgr.h"
#include "runtime/ErrorMgr.h"
#include "runtime/Socks.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/LogComment.h"
#include "runtime/Connection.h"
#include "runtime/ConnMgr.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/globals.h"

#include "runtime/SslWrap.h"

int Connection::TheLastSeqId = 0;


/* Connection::HalfPipe */

void Connection::HalfPipe::reset() {
	Assert(!theReserv);
	theBuf.reset();
	theIOCnt = 0;
	isReady = false;
}

void Connection::HalfPipe::start(FileScanUser *u) {
	Should(!theReserv);
	theReserv = TheFileScanner->setFD(theConn.theSock.fd(), theDir, u);
}

void Connection::HalfPipe::start(FileScanUser *u, Time timeout) {
	start(u);
	TheFileScanner->setTimeout(theReserv.fd(), timeout);
}

void Connection::HalfPipe::stop(FileScanUser *u) {
	if (!u || TheFileScanner->user(theReserv, u)) {
		if (theReserv)
			TheFileScanner->clearRes(theReserv);
		theBuf.pack();
	}
}

void Connection::HalfPipe::changeUser(FileScanUser *uOld, FileScanUser *uNew) {
	if (theReserv)
		TheFileScanner->changeUser(theReserv, uOld, uNew);
}


/* Connection::NtlmAuth */

void Connection::NtlmAuth::reset() {
	state = ntlmNone;
	useSpnegoNtlm = false;
	hdrRcvdT2 = String();
	gssContext.free();
}

/* Connection::LayeredSsl */

bool Connection::LayeredSsl::addSsl(Ssl *ssl) {
	Should(ssl);
	if (!theSsl[lastLayer])
		theSsl[lastLayer] = ssl;
	else if (!lastLayer)
		theSsl[++lastLayer] = ssl;
	else
		return false;
	return true;
}

bool Connection::LayeredSsl::dataPending() const {
	return theSsl[0]->dataPending() || (lastLayer && theSsl[1]->dataPending());
}

Size Connection::LayeredSsl::read(char *buf, Size sz) {
	return netLayer()->read(buf, sz);
}

Size Connection::LayeredSsl::write(const char *buf, Size sz) {
	return netLayer()->write(buf, sz);
}

bool Connection::LayeredSsl::resumeSession(SslSession *session) {
	return netLayer()->resumeSession(session);
}

bool Connection::LayeredSsl::reusedSession() const {
	return netLayer()->reusedSession();
}

// XXX: Needs more work to identify the original error
// because the other layer may be the true cause of an error.
int Connection::LayeredSsl::getError(int e) {
	return netLayer()->getError(e);
}

// XXX: Needs more work to identify the original error
// because the other layer may be the true cause of an error.
const char *Connection::LayeredSsl::getErrorString(int e) {
	return netLayer()->getErrorString(e);
}

bool Connection::LayeredSsl::shutdown(int &res) {
	int i = lastLayer;
	do {
		if (!theSsl[i]->shutdown(res))
			return false;
	} while (i--);
	return true;
}

void Connection::LayeredSsl::reset() {
	theSsl[0] = 0;
	theSsl[1] = 0;
	lastLayer = 0;
}

void Connection::LayeredSsl::close() {
	do {
		delete theSsl[lastLayer];
	} while (lastLayer--);
	reset();
}

bool Connection::LayeredSsl::setIO(int fd) {
	Should(netLayer()->enablePartialWrite());
	Should(netLayer()->acceptMovingWriteBuffer()); // just in case

	return lastLayer ? theSsl[lastLayer]->setBIO(theSsl[lastLayer - 1]) :
	               theSsl[lastLayer]->setFd(fd);
}


/* Connection */

Connection::Connection():
	protoStat(0),
	theRd(*this, theRdBuf, dirRead),
	theWr(*this, theWrBuf, dirWrite),
	usedSsl(false),
	usedSocks(false),
	theSocks(0), theSsl() {
	reset();
}

void Connection::reset() {
	protoStat = 0;

	theProxyNtlmState.reset();
	theOriginNtlmState.reset();

	if (theSock || theSsl)
		closeNow();

	usedSsl = usedSocks = false;

	theAddr = NetAddr();
	theMgr = 0;
	thePortMgr = 0;
	theSslCtxOrigin = 0;
	theSslCtxProxy = 0;
	theSslSessionOrigin = 0;
	theSslSessionProxy = 0;
	theSsl.reset();
	theRd.reset();
	theWr.reset();
	theCloseKind = ConnCloseStat::ckNone;
	theLogCat = lgcAll;
	isBad = isAtEof = isLastUse = isFirstUse = isSocksAuthed =
		isSslEstablished = wasAnnounced = isTunnelEstablished = false;

	theSocksProxy = theTunnel = NetAddr();
	theLocPort = theRemPort = -1;

	theUseCountLmt = theUseLevelLmt = -1; // unlimited use by default
	theUseCnt = theUseLvl = theUseLvlMax = 0;
	theOpenTime = theUseStart = Time();
	theMaxIoSize = -1;

	if (theSocks)
		socksEnd();

	theSeqId = ++TheLastSeqId;
}

void Connection::useSsl(const SslCtx *aCtx, SslSession *aSession) {
	theSslCtxOrigin = aCtx;
	theSslSessionOrigin = aSession;
}

void Connection::useSslProxy(const SslCtx *aCtx, SslSession *aSession) {
	theSslCtxProxy = aCtx;
	theSslSessionProxy = aSession;
}

NetAddr Connection::laddr() const {
	NetAddr addr;
	if (theLocPort >= 0 && thePortMgr) {
		addr = thePortMgr->addr();
		addr.port(theLocPort);
	}
	return addr;
}

int Connection::rport() const {
	if (!theSock)
		return -1;
	else
	if (theRemPort >= 0)
		return theRemPort;
	else
		return theRemPort = theSock.rport(); // cache the result
}

bool Connection::connect(const NetAddr &addr, const SockOpt &opt, PortMgr *aPortMgr, const NetAddr &socksProxy) {
	theAddr = addr;
	theSocksProxy = socksProxy;
	Assert(aPortMgr);
	Assert(!theSock);
	Assert(!wasAnnounced);
	const NetAddr hop_addr = theSocksProxy ? theSocksProxy : theAddr;
	if (!theSock.create(hop_addr.addrN().family())) {
		ReportError(Error::Last());
		return false;
	}

	if (!setSockOpt(opt))
		return false;

	thePortMgr = aPortMgr;
	theLocPort = thePortMgr->bind(theSock);
	if (theLocPort < 0)
		return false;

	theOpenTime = TheClock;
	Broadcast(TheConnOpenChannel, this);
	wasAnnounced = true;

	if (!theSock.connect(hop_addr)) {
		ReportError(Error::Last());
		return false;
	}

	isFirstUse = true;

	if (theSocksProxy)
		socksStart();
	else if (theSslCtxProxy)
		sslActivateProxy();

	// SSL to origin is not activated implicitly, call sslActivate() when needed
	return true; 
}

const Ssl *Connection::sslActive() const {
	return theSslCtxProxy && theSslCtxOrigin ? theSsl.activeSecond() : theSsl.activeFirst();
}

const Ssl *Connection::sslActiveProxy() const {
	return theSslCtxProxy ? theSsl.activeFirst() : 0;
}

bool Connection::sslActivate() {
	return Should(theSslCtxOrigin) && sslConnect(false);
}

bool Connection::sslActivateProxy() {
	if (Should(theSslCtxProxy) && sslConnect(true)) {
		Should(theSsl.active());
		return true;
	}
	return false;
}

bool Connection::accept(Socket &s, const SockOpt &opt, bool &fatal) {
	Assert(!theSock);
	Assert(!wasAnnounced);

	theSock = s.accept(theAddr);
	if (!theSock) {
		const Error err = Error::Last();
		fatal = false;
		if (err != EWOULDBLOCK && err != EAGAIN && err != EMFILE)
			ReportError(err);
		return false;
	}

	if (!setSockOpt(opt))
		return false;

	theLocPort = -1; // unknown, do not care;
	theOpenTime = TheClock;

	// now that we are connected, maybe associate the socket with SSL state
	if (theSslCtxOrigin && !sslAccept())
		return false;

	Broadcast(TheConnOpenChannel, this);
	wasAnnounced = true;

	isFirstUse = true;

	return true;
}

// zero read does not mean EOF because SSL may buffer
// check bad() and atEof()
// TODO: collect SSL and SOCKS stats
Size Connection::read() {
	if (!theRdBuf.spaceSize() || !theMaxIoSize) {
		theRd.isReady = true;
		return 0;
	}
	theRd.isReady = false;

	const Size ioSz = theMaxIoSize >= 0 ?
		Min(theRdBuf.spaceSize(), theMaxIoSize) : theRdBuf.spaceSize();
	Size sz;
	if (!preIo(theRd, "read after accept"))
		sz = 0;
	else
	if (theSocks)
		sz = socksRead();
	else
	if (theSsl)
		sz = sslRead(ioSz);
	else
		sz = rawRead(ioSz);
	theMaxIoSize = -1;

	//cerr << here << "read " << sz << " : {" << theRdBuf.space() << "}" << endl;

	if (isFirstUse)
		isFirstUse = false;

	if (sz > 0) {
		theRdBuf.appended(sz);
		TheStatPhaseMgr->noteSockRead(sz, logCat());
		return sz;
	}

	return 0;
}

// TODO: collect SSL and SOCKS stats
Size Connection::write() {
	if (!theWrBuf.contSize() || !theMaxIoSize) {
		theWr.isReady = true;
		return 0;
	}
	theWr.isReady = false;

	const Size ioSz = theMaxIoSize >= 0 ?
		Min(theWrBuf.contSize(), theMaxIoSize) : theWrBuf.contSize();
	Size sz;
	if (!preIo(theWr, "write after connect"))
		sz = 0;
	else
	if (theSocks)
		sz = socksWrite();
	else
	if (theSsl)
		sz = sslWrite(ioSz);
	else
		sz = rawWrite(ioSz);
	theMaxIoSize = -1;
	
	//cerr << here << "wrote " << sz << " : {" << theWrBuf.content() << "}" << endl;

	if (sz >= 0) {
		TheStatPhaseMgr->noteSockWrite(sz, logCat());
		if (sz > 0) {
			theWrBuf.consumed(sz);
			theWrBuf.pack();
			if (isFirstUse)
				isFirstUse = false;
			return sz;
		}
	}

	return 0;
}

bool Connection::closeNow() {
	bool res = true;

	if (theSsl)
		res = sslCloseNow() && res;

	return rawCloseNow() && res;
}

// does not close raw socket
bool Connection::closeAsync(FileScanUser *u, bool &fatal) {
	if (theSsl)
		return sslCloseAsync(u, fatal);
	else
		return true;
}

bool Connection::sslSessionReused() const {
	return Should(theSsl) && theSsl.reusedSession();
}

void Connection::sslStart(int role, const bool toProxy) {
	const SslCtx *sslCtx = toProxy ? theSslCtxProxy : theSslCtxOrigin;
	SslSession *sslSession = toProxy ? theSslSessionProxy : theSslSessionOrigin;
	usedSsl = true;
	Ssl *ssl = sslCtx->makeConnection();
	ssl->playRole(role); // // must set mode before setting fd
	if (sslSession) {
		if (!ssl->resumeSession(sslSession) &&
			ReportError(errSslSessionResume)) {
			print(Comment << "on connection ");
			SslWrap::PrintErrors(Comment) << endc;
		}
	}
	Should(theSsl.addSsl(ssl));
	Should(theSsl.setIO(theSock.fd()));

	if (sslToOrigin() && Should(theSsl.active()))
		Broadcast(TheConnSslActiveChannel, this);
}

bool Connection::sslConnect(const bool toProxy) {
	sslStart(Ssl::rlClient, toProxy);
	return true;
}

bool Connection::sslAccept() {
	sslStart(Ssl::rlServer, false);
	return true;
}

// zero read does not mean EOF
Size Connection::sslRead(Size ioSz) {
	Should(!isAtEof);
	const Size sz = theSsl.read(theRdBuf.space(), ioSz);
	//clog << here << "ssl read: " << sz << " ? " << theSsl->dataPending() << endl;

	if (sz > 0) {
		if (theSsl.dataPending())
			TheFileScanner->forceReady(theSock.fd());
		if (!isSslEstablished && sslToOrigin()) {
			Broadcast(TheConnSslEstablishedChannel, this);
			isSslEstablished = true;
		}
		return sz;
	}
	//clog << here << this << "ssl err fd: " << theSock.fd() << ' ' << theSsl->getErrorString(sz) << endl;

	switch (theSsl.getError(sz)) {
		case SSL_ERROR_ZERO_RETURN:
			isAtEof = true;
			break;
		case SSL_ERROR_WANT_READ:
			// OK, will try again later
			break;
		case SSL_ERROR_SYSCALL:
			if (sz == 0) {
				isAtEof = true;
				sslError(sz, "protocol-violating EOF on read");
			} else {
				rawError("read on SSL connection");
			}
			break;
		case SSL_ERROR_WANT_WRITE:
			TheFileScanner->setReadNeedsWrite(theSock.fd());
			break;
		default:
			sslError(sz, "read");
	}

	return 0;
}

Size Connection::sslWrite(Size ioSz) {
	const Size sz = theSsl.write(theWrBuf.content(), ioSz);
	//clog << here << "ssl wrote: " << sz << endl;

	if (sz > 0) {
		if (!isSslEstablished && sslToOrigin()) {
			Broadcast(TheConnSslEstablishedChannel, this);
			isSslEstablished = true;
		}
		return sz;
	}
	//clog << here << this << "ssl err fd: " << theSock.fd() << ' ' << theSsl->getErrorString(sz) << endl;

	switch (theSsl.getError(sz)) {
		case SSL_ERROR_ZERO_RETURN:
			// not a connection-level error; XXX: caller must handle!
			isAtEof = true;
			break;
		case SSL_ERROR_WANT_WRITE:
			// OK, will try again later
			break;
		case SSL_ERROR_SYSCALL:
			if (sz == 0)
				sslError(sz, "protocol-violating EOF on write");
			else
				rawError("write on SSL connection");
			break;
		case SSL_ERROR_WANT_READ:
			TheFileScanner->setWriteNeedsRead(theSock.fd());
			break;
		default:
			sslError(sz, "write");
	}

	return 0;
}

bool Connection::sslCloseNow() {
	// assume bi-directional shutdown is not needed
	int err;
	const bool res = theSsl.shutdown(err) || err == 0;
	if (!res)
		sslError(err, "close");

	sslForget();
	return res;
}

bool Connection::sslCloseAsync(FileScanUser *u, bool &fatal) {
	int err;
	// assume bi-directional shutdown is not needed
	if (theSsl.shutdown(err) || err == 0) {
		sslForget();
		return true;
	}

	Should(err < 0);
	switch (theSsl.getError(err)) {
		case SSL_ERROR_WANT_WRITE:
			theRd.stop(0);
			if (!theWr.theReserv)
				theWr.start(u);
			fatal = false;
			break;
		case SSL_ERROR_WANT_READ:
			theWr.stop(0);
			if (!theRd.theReserv)
				theRd.start(u);
			fatal = false;
			break;
		case SSL_ERROR_SYSCALL:
			rawError("close on SSL connection");
			fatal = true;
			break;
		default:
			sslError(err, "close");
			fatal = true;
	}

	if (fatal)
		sslForget();

	return false;
}


void Connection::sslForget() {
	if (theSslCtxOrigin)
		Broadcast(TheConnSslInactiveChannel, this);
	theSsl.close();
}

void Connection::sslError(int err, const char *operation) {
	if (ReportError(errSslIo)) {
		Comment << "error: SSL " << operation
			<< " failure with err=" << err
			<< '/' << theSsl.getErrorString(err)
			<< '/' << Error::Last().no()
			<< std::endl;
		SslWrap::PrintErrors(Comment);
		print(Comment << "on connection ") << endc;
	}
	isBad = true;
}

void Connection::socksStart() {
	if (!Should(!theSocks))
		socksEnd();
	usedSocks = true;
	theSocks = new Socks(*this);
	TheFileScanner->setReadNeedsWrite(theSock.fd());
}

void Connection::socksEnd() {
	if (!Should(theSocks))
		return;
	isSocksAuthed = theSocks->authed();
	delete theSocks;
	theSocks = 0;
}

bool Connection::socksAuthed() const {
	return theSocks ? theSocks->authed() : isSocksAuthed;
}

Size Connection::socksRead() {
	return socksOp(sopRead);
}

Size Connection::socksWrite() {
	return socksOp(sopWrite);
}

Size Connection::socksOp(const SocksOp op) {
	Assert(theSocks);
	const Socks::Result res = theSocks->perform();
	switch (res) {
		case Socks::resWantRead:
			if (op == sopWrite)
				TheFileScanner->setWriteNeedsRead(theSock.fd());
			break;
		case Socks::resWantWrite:
			if (op == sopRead)
				TheFileScanner->setReadNeedsWrite(theSock.fd());
			break;
		case Socks::resCompleted:
			socksEnd();
			break;
		default:
			isBad = true;
	}
	return 0; // we have read or written SOCKS data, not HTTP
}

// zero read means EOF
Size Connection::rawRead(Size ioSz) {
	const Size sz = theSock.read(theRdBuf.space(), ioSz);

	if (sz > 0)
		return sz;

	if (sz == 0)
		isAtEof = true;
	else
	if (sz < 0 && Error::Last() != EWOULDBLOCK)
		rawError("read");

	return 0;
}

Size Connection::rawWrite(Size ioSz) {
	const Size sz = theSock.write(theWrBuf.content(), ioSz);

	if (sz > 0)
		return sz;

	if (sz < 0 && Error::Last() != EWOULDBLOCK)
		rawError("write");

	return 0;
}


bool Connection::rawCloseNow() {
	Should(!theSsl); // must be close by now

	if (theLocPort >= 0) {
		thePortMgr->release(theLocPort, true);
		theLocPort = -1;
	}

	bool res = true;
	if (theSock) {
		theRd.stop(0);
		theWr.stop(0);
		if (wasAnnounced)
			Broadcast(TheConnCloseChannel, this);

		if (!theSock.close()) {
			res = false;
			rawError("close");
		}
		if (isFirstUse)
			isFirstUse = false;
	}

	return res;
}


void Connection::rawError(const char *operation) {
	if (ReportError(Error::Last())) {
		print(Comment(3) << "error: raw " << operation <<
			" failed on connection ") << endc;
	}
	isBad = true;
}

std::ostream &Connection::print(std::ostream &os) const {
	if (const NetAddr local = laddr())
		os << "between " << local << " and " << raddr();
	else
		os << "with " << raddr();
	if (theSocksProxy)
		os << " through SOCKS proxy " << theSocksProxy;
	os << " at " << theRd.theIOCnt << " reads, "
		<< theWr.theIOCnt << " writes, and "
		<< useCnt() << " transactions";
    return os;
}

void Connection::decMaxIoSize(Size aMax) {
	if (theMaxIoSize < 0)
		theMaxIoSize = aMax;
	else
		theMaxIoSize = Min(theMaxIoSize, aMax);
}

// this has to be done for accepted sockets as well
// FreeBSD accept(2) man page misleads
bool Connection::setSockOpt(const SockOpt &opt) {
	return Should(theSock.blocking(false) && theSock.configure(opt));
}

bool Connection::preIo(HalfPipe &ioPipe, const char *operation) {
	if (ioCnt() == 0) {
		// check for connect/accept errors
		if (theSock.error()) {
			rawError(operation);
			return false;
		}
		Broadcast(TheConnEstChannel, this);
	}
	ioPipe.theIOCnt++;
	return true;
}

bool Connection::reusable() const {
	if (exhausted() || isLastUse || isBad)
		return false;
	return theUseCountLmt < 0 || theUseCnt < theUseCountLmt;
}

bool Connection::pipelineable() const {
	if (!reusable())
		return false;
	return theUseLevelLmt < 0 || theUseLvl < theUseLevelLmt;
}

// prepare for next use if possible
// note: the notion of ``use'' is caller defined
void Connection::startUse() {
	Assert(reusable());
	// first user takes the hit of the opening delay
	theUseStart = theUseCnt++ ? (Time)TheClock : theOpenTime;

	++theUseLvl;
	if (theUseLvl > theUseLvlMax)
		theUseLvlMax = theUseLvl;
}

void Connection::finishUse() {
	Assert(theUseLvl > 0);
	--theUseLvl;
}

bool Connection::hasCredentials() const {
	return theMgr && theMgr->hasCredentials();
}

bool Connection::genCredentials(UserCred &cred) const {
	return theMgr && theMgr->credentialsFor(*this, cred);
}

void Connection::setTunnelEstablished() {
	if (ShouldUs(!isTunnelEstablished)) {
		isTunnelEstablished = true;
		Broadcast(TheConnTunnelEstablishedChannel, this);
	}
}
