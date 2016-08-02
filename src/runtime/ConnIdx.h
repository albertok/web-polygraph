
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_CONNIDX_H
#define POLYGRAPH__RUNTIME_CONNIDX_H

#include "xstd/Array.h"


// fd -> connection map
// we can make this map generic (fd->something) map when needed

class Connection;

class ConnIdx: protected Array<Connection*> {
	public:
		void fdLimit(int limit);

		void add(Connection *conn);
		void del(Connection *conn);

		// asserts that conn belongs to the index
		void check(const Connection *conn);

		Connection *operator [](int fd) const { return item(fd); }

	protected:
		// checks that conn's place is occupied by val and returns the "place"
		Connection *&safeItem(const Connection *conn, const Connection *val);
};

#endif
