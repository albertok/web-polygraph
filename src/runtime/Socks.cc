
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/Connection.h"
#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"
#include "runtime/UserCred.h"

#include "runtime/Socks.h"


// constants for SOCKS5 protocol
enum {
	SocksVersion = 0x05,
	AuthUserPassVersion = 0x01,

	EstablishTcpConnection = 0x01,

	AddressTypeIpV4 = 0x01,
	AddressTypeDomainName = 0x03,
	AddressTypeIpV6 = 0x04,

	StatusSuccess = 0x00,
	StatusGeneralFailure = 0x01,
	StatusConnectionNotAllowedByRuleset = 0x02,
	StatusNetworkUnreachable = 0x03,
	StatusHostUnreachable = 0x04,
	StatusConnectionRefusedByDestinationHost = 0x05,
	StatusTtlExpired = 0x06,
	StatusCommandNotSupported = 0x07,
	StatusAddressTypeNotSupported = 0x08,
	StatusMax = 0x09
};

static const char *const StatusStrings[] = {
	"succeeded",
	"general SOCKS server failure",
	"connection not allowed by ruleset",
	"network unreachable",
	"host unreachable",
	"connection refused",
	"TTL expired",
	"command not supported",
	"address type not supported",
	"unassigned"
};

struct ConnectRequest {
	uint8_t version;
	uint8_t command;
	uint8_t reserved;
	uint8_t addressType;
};


Socks::Socks(Connection &aConn):
	theConn(aConn),
	theAuth(authInvalid),
	theStatus(StatusMax),
	theState(stConnected) {
}

Socks::Result Socks::perform() {
	const State oldState = theState;
	Error error;
	if (theWrBuf.empty()) {
		switch (theState) {
			case stConnected:
				error = makeGreeting();
				break;
			case stGreetingSent:
				error = recvGreetingResponse();
				break;
			case stGreetingCompleted:
				error = makeAuth();
				break;
			case stAuthSent:
				error = recvAuthResponse();
				break;
			case stAuthCompleted:
				error = makeConnectRequest();
				break;
			case stConnectRequestSent:
				error = recvConnectResponse();
				break;
			case stCompleted:
				return resCompleted;
			case stError:
				return resError;
			default:
				Assert(false);
		}
	}

	Result res = resError;
	if (!error) {
		if (theWrBuf.empty()) {
			// read operation
			res = oldState == theState ? resWantRead :
				(theState == stCompleted ? resCompleted :
				resWantWrite);
		} else {
			// write operation
			error = send();
			res = theWrBuf.empty() ? resWantRead : resWantWrite;
		}
	}
	if (error) {
		reportError(error);
		theState = stError;
	}
	return res;
}

Error Socks::send() {
	Assert(!theWrBuf.empty());

	const Size sz = theConn.sock().write(theWrBuf.content(), theWrBuf.contSize());
	Error error;
	if (sz >= 0)
		theWrBuf.consumed(sz);
	else
	if (Error::Last() != EWOULDBLOCK)
		error = errSocksWrite;
	return error;
}

Error Socks::makeGreeting() {
	Size msg_size = theConn.hasCredentials() ? 4 : 3;
	Error error;
	if (theWrBuf.spaceSize() >= msg_size) {
		char *const p = theWrBuf.space();
		p[0] = SocksVersion;
		p[1] = theConn.hasCredentials() ? 2 : 1;
		p[2] = authNo;
		if (theConn.hasCredentials())
			p[3] = authUserPass;
		theWrBuf.appended(msg_size);
		theState = stGreetingSent;
	} else
		error = errSocksReqDontFit;
	return error;
}

Error Socks::makeAuth() {
	UserCred cred;
	Assert(theConn.genCredentials(cred));

	Assert(theAuth == authUserPass);
	const Size msg_size = 2 + cred.image().len();
	Error error;
	if (theWrBuf.spaceSize() >= msg_size) {
		char *const p = theWrBuf.space();
		p[0] = AuthUserPassVersion;
		const char name_length = Min(0xFF, cred.name().size());
		p[1] = name_length;
		theWrBuf.appended(2);
		theWrBuf.append(cred.name().data(), name_length);
		const char password_length = Min(0xFF, cred.password().size());
		*theWrBuf.space() = password_length;
		theWrBuf.appended(1);
		theWrBuf.append(cred.password().data(), password_length);
		theState = stAuthSent;
	} else
		error = errSocksReqDontFit;
	return error;
}

// TODO: Can we append Connection write buffer? It seems wrong to lose one I/O
// if SOCKS handshake and raw content can be combined.
Error Socks::makeConnectRequest() {
	Error error;
	if (theWrBuf.spaceSize() >= connectMessageSize()) {
		ConnectRequest *const request =
			reinterpret_cast<ConnectRequest *>(theWrBuf.space());
		request->version = SocksVersion;
		request->command = EstablishTcpConnection;
		request->reserved = 0x00;
		theWrBuf.appended(sizeof(*request));
		if (theConn.raddr().isDomainName()) {
			request->addressType = AddressTypeDomainName;
			const String &addrA = theConn.raddr().addrA();
			*theWrBuf.space() = addrA.len();
			theWrBuf.appended(1);
			theWrBuf.append(addrA.data(), addrA.len());
		} else {
			const InAddress &addrN = theConn.raddr().addrN();
			request->addressType = addrN.family() == AF_INET6 ?
				AddressTypeIpV6 : AddressTypeIpV4;
			theWrBuf.append(reinterpret_cast<const char *>(addrN.rawOctets()), addrN.len());
		}
		uint16_t *const port =
			reinterpret_cast<uint16_t *>(theWrBuf.space());
		*port = htons(theConn.raddr().port());
		theWrBuf.appended(sizeof(*port));

		theState = stConnectRequestSent;
	} else
		error = errSocksReqDontFit;
	return error;
}

Error Socks::recv(const Size size, const bool checkVersion) {
	Assert(size > 0);
	Error error;
	if (theRdBuf.capacity() < size)
		error = errSocksRepDontFit;
	else
	if (Should(theRdBuf.contSize() < size)) {
		const Size sz =
			theConn.sock().read(theRdBuf.space(), size - theRdBuf.contSize());
		if (sz > 0)
			theRdBuf.appended(sz);
		else
		if (sz == 0)
			error = errSocksUnexpectedEof;
		else
		if (Error::Last() != EWOULDBLOCK)
			error = errSocksRead;
	}
	if (checkVersion && !error && *theRdBuf.content() != SocksVersion)
		error = errSocksVersion;
	return error;
}

Error Socks::recvGreetingResponse() {
	static const Size msg_size = 2;
	Error error = recv(msg_size);
	if (!error && theRdBuf.contSize() == msg_size) {
		theAuth = theRdBuf.content()[1];
		switch(theAuth) {
			case authNo:
				break;
			case authUserPass:
				if (!theConn.hasCredentials())
					error = errSocksAuthUserPassWoutCreds;
				break;
			case authInvalid:
				error = errSocksAuthInvalid;
				break;
			default:
				error = errSocksAuthUnsupported;
		}
		theRdBuf.consumed(msg_size);
		theState = theAuth == authNo ? stAuthCompleted : stGreetingCompleted;
	}
	return error;
}

Error Socks::recvAuthResponse() {
	static const Size msg_size = 2;
	Error error = recv(msg_size, false);
	if (!error && theRdBuf.contSize() == msg_size) {
		if (theRdBuf.content()[0] != AuthUserPassVersion)
			error = errSocksAuthUserPassVersion;
		else
		if (theRdBuf.content()[1] != StatusSuccess)
			error = errSocksAuthForbidden;
		theRdBuf.consumed(msg_size);
		theState = stAuthCompleted;
	}
	return error;
}

Error Socks::recvConnectResponse() {
	Error error = recv(connectMessageSize());
	if (!error && theRdBuf.contSize() == connectMessageSize()) {
		theStatus = theRdBuf.content()[1];
		if (theStatus != StatusSuccess)
			error = errSocksConnect;
		theRdBuf.consumed(connectMessageSize());
		theState = stCompleted;
	}
	return error;
}

int Socks::connectMessageSize() const {
	return sizeof(ConnectRequest) + theConn.raddr().addrN().len() + 2;
}

void Socks::reportError(const Error &error) const {
	if (Should(error) && ReportError2(error, theConn.logCat())) {
		if (error == errSocksRead)
			Comment << "SOCKS read error: " << Error::Last() << endl;
		else
		if (error == errSocksWrite)
			Comment << "SOCKS write error: " << Error::Last() << endl;
		else
		if (error == errSocksConnect) {
			const int index =
				Min(theStatus, static_cast<char>(StatusMax));
			Comment << "connect status: " << StatusStrings[index] << endl;
		}
		theConn.print(Comment << "on connection ") << endc;
	}
}
