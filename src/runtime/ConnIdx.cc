
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/Connection.h"
#include "runtime/ConnIdx.h"


void ConnIdx::fdLimit(int limit) {
	Assert(count() <= limit);
	resize(limit);
}

void ConnIdx::add(Connection *conn) {
	safeItem(conn, 0) = conn;
}

void ConnIdx::del(Connection *conn) {
	safeItem(conn, conn) = 0;
}

void ConnIdx::check(const Connection *conn) {
	(void)safeItem(conn, conn);
}

Connection *&ConnIdx::safeItem(const Connection *conn, const Connection *val) {
	Assert(conn);
	const int fd = conn->fd();
	Assert(fd >= 0 && fd < count());
	Connection *&res = item(fd);
	Assert(res == val);
	return res;
}
