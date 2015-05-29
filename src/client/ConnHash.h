
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CONNHASH_H
#define POLYGRAPH__CLIENT_CONNHASH_H

#include "xstd/Array.h"
#include "runtime/Farm.h"
#include "runtime/Connection.h"

// address -> connection map

class NetAddr;
class ConnHashItem;

typedef ConnHashItem **ConnHashPos;

struct ConnHashQueueItem {
	Connection *conn;

	ConnHashQueueItem(): conn(0) {}
	void reset() { conn = 0; }

	QueuePlace<ConnHashQueueItem> connHashes;
};

typedef Queue<ConnHashQueueItem, &ConnHashQueueItem::connHashes> ConnHashQueue;

class ConnHashItem {
	public:
		ConnHashItem();

		void reset();

		bool vacant() const;
		bool matches(const NetAddr &addr, const NetAddr &tunnelAddr) const;

		ConnHashQueue &queue() { return theQueue; }
		ConnHashItem *collision() { return theCollision; }
		ConnHashPos nextPos() { return &theCollision; }

	protected:
		ConnHashQueue theQueue;
		ConnHashItem *theCollision;
};

class ConnHash {
	public:
		ConnHash(int ccAddrMax = 0);

		void ccAddrMax(int aMax);

		void add(Connection *conn);
		void del(Connection *conn);
		Connection *delAt(ConnHashPos pos);
		bool find(const NetAddr &addr, const NetAddr &tunnelAddr);
		bool find(const NetAddr &addr, const NetAddr &tunnelAddr, ConnHashPos &pos);

	protected:
		int hash(const NetAddr &addr) const;
		void delAt(ConnHashPos pos, ConnHashQueueItem *qi);

	protected:
		static ObjFarm<ConnHashItem> TheItems;
		static ObjFarm<ConnHashQueueItem> TheQueueItems;
		Array<ConnHashItem*> theHash;
};

#endif
