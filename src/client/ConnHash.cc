
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/ConnHash.h"
#include "xstd/gadgets.h"


ObjFarm<ConnHashItem> ConnHash::TheItems;
ObjFarm<ConnHashQueueItem> ConnHash::TheQueueItems;


/* ConnHashItem */

ConnHashItem::ConnHashItem(): theCollision(0) {
}

void ConnHashItem::reset() {
	theCollision = 0;
}

bool ConnHashItem::vacant() const {
	return theQueue.count() == 0;
}

bool ConnHashItem::matches(const NetAddr &hopAddr, const NetAddr &tunnelAddr) const {
	Assert(theQueue.count());
	const Connection *conn = theQueue.firstOut()->conn;
	// if tunnelAddr is undefined, check that connection tunnel end is as well
	return conn->raddr() == hopAddr && conn->tunnelEnd() == tunnelAddr;
}


/* ConnHash */

ConnHash::ConnHash(int anCcAddrMax) {
	if (anCcAddrMax)
		ccAddrMax(anCcAddrMax);
}

void ConnHash::ccAddrMax(int aMax) {
	theHash.resize(Max(aMax|1, 7));
	TheItems.limit(1024);      // magic, no good way to estimate
	TheQueueItems.limit(1024); // magic, no good way to estimate
}

int ConnHash::hash(const NetAddr &addr) const {
	return addr.addrN().hash0() % theHash.count();
}

bool ConnHash::find(const NetAddr &hopAddr, const NetAddr &tunnelAddr, ConnHashPos &pos) {
	const int idx = hash(hopAddr);

	pos = &theHash[idx];
	while (ConnHashItem *hi = *pos) {
		if (hi->matches(hopAddr, tunnelAddr))
			return true;
		pos = hi->nextPos();
	}
	return false;
}

bool ConnHash::find(const NetAddr &hostAddr, const NetAddr &tunnelAddr) {
	ConnHashPos pos;
	return find(hostAddr, tunnelAddr, pos);
}

void ConnHash::add(Connection *conn) {
	ConnHashPos pos;
	if (!find(conn->raddr(), conn->tunnelEnd(), pos))
		*pos = TheItems.get();
	ConnHashQueueItem *qi = TheQueueItems.get();
	qi->conn = conn;
	(*pos)->queue().enqueue(qi);
}

void ConnHash::delAt(ConnHashPos pos, ConnHashQueueItem *qi) {
	TheQueueItems.put(qi);
	ConnHashItem *hi = *pos;
	if (hi->vacant()) {
		*pos = hi->collision();
		TheItems.put(hi);
	}
}

Connection *ConnHash::delAt(ConnHashPos pos) {
	Assert(*pos);
	ConnHashItem *hi = *pos;
	ConnHashQueueItem *qi = hi->queue().dequeue(hi->queue().lastIn()); // LIFO
	Connection *conn = qi->conn;
	delAt(pos, qi);
	return conn;
}

void ConnHash::del(Connection *conn) {
	ConnHashPos pos;
	Assert(find(conn->raddr(), conn->tunnelEnd(), pos));
	ConnHashItem *hi = *pos;
	// XXX: this loop may be expensive if queues get long
	// perhaps merge ConnHashQueueItem with Connection somehow?
	for (ConnHashQueueItem *qi = hi->queue().begin();
		qi != hi->queue().end();
		qi = hi->queue().next(qi)) {
		if (qi->conn == conn) {
			hi->queue().dequeue(qi);
			delAt(pos, qi);
			return;
		}
	}
	Assert(false);
}
