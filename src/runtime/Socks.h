
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_SOCKS_H
#define POLYGRAPH__RUNTIME_SOCKS_H

#include "runtime/IOBuf.h"

class Connection;


// handles connection over SOCKS5 proxy
class Socks {
	public:
		Socks(Connection &aConn);

		enum Result { resWantWrite, resWantRead, resCompleted, resError };
		Result perform();

	private:
		enum Auth {
			authNo = '\x00', // no authentication
			authUserPass = '\x02', // username/password authentication
			authInvalid = '\xFF' // no acceptable methods
		};
		enum State {
			stConnected, // connection to the proxy established
			stGreetingSent, // greeting sent
			stGreetingCompleted, // authentication should be sent
			stAuthSent, // authentication sent
			stAuthCompleted, // authentication completed
			stConnectRequestSent, // connect request sent
			stCompleted, // connection to the peer established
			stError // error occured
		};

	public:
		bool authed() const { return theState == stCompleted && theAuth == authUserPass; }

	private:
		Error send();
		Error makeGreeting();
		Error makeAuth();
		Error makeConnectRequest();

		Error recv(const Size size, const bool checkVersion = true);
		Error recvGreetingResponse();
		Error recvAuthResponse();
		Error recvConnectResponse();

		int connectMessageSize() const;

		void reportError(const Error &error) const;

		Connection &theConn; // underlying connection
		IOBuf theRdBuf; // input buffer
		IOBuf theWrBuf; // output buffer
		char theAuth; // authentication method selected by server
		char theStatus; // reply status
		State theState; // state of the SOCKS connection
};

#endif
