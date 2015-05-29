
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "runtime/IOBuf.h"
#include "runtime/httpText.h"
#include "client/CltDataFilterRegistry.h"
#include "client/UserCred.h"
#include "client/CltXact.h"

class HttpBasicAuthenticator: public CltDataFilterRegistry::Filter {
	public:
		virtual String id() const { return "HttpBasicAuthenticator-0.1"; }
		virtual void describe(ostream &os) const;

		virtual void apply(CltDataFilterRegistry::Producer &p, IOBuf &buf);
};

static bool registered = registered ||
	TheCltDataFilterRegistry().add(new HttpBasicAuthenticator);


void HttpBasicAuthenticator::describe(ostream &os) const {
	os << "adds HTTP Basic Authentication headers";
}

void HttpBasicAuthenticator::apply(CltDataFilterRegistry::Producer &p, IOBuf &buf) {
	ofixedstream os(buf.space(), buf.spaceSize());

	static const String hfpInitiatedBy = "Initiated-By: ";
	const char *ip = p->conn()->laddr().addrN().image();
	os << hfpInitiatedBy << ip << crlf;

	const UserCred &credentials = p->credentials();
	if (!credentials.image())
		return;

	PrintBase64(os << hfpProxyAuthorizationBasic,
		credentials.image().data(), credentials.image().len()) << crlf;
	buf.appended((std::streamoff)os.tellp());
}
