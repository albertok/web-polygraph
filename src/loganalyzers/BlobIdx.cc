
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "xstd/Checksum.h"
#include "xstd/gadgets.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobIdx.h"

static const String strKey = "key";

BlobIdx::BlobIdx(int aCapacity): theCount(0) {
	theHash.resize(aCapacity);
}

void BlobIdx::add(const ReportBlob *blob) {
	Assert(blob);

	while (2*theHash.capacity() < 3*theCount)
		grow();

	int idx;
	Assert(!find(blob->key(), idx));
	theHash.put(blob, idx);
	theCount++;
}

const ReportBlob *BlobIdx::find(const Key &key) const {
	int idx;
	return find(key, idx);
}

const ReportBlob *BlobIdx::find(const Key &key, int &idx) const {

	if (!theHash.capacity()) {
		idx = 0;
		return 0;
	}

	xstd::ChecksumAlg alg;
	alg.update(key.data(), key.len());
	alg.final();
	static unsigned int hashVals[4];
	memcpy(hashVals, alg.sum().image(), Min(SizeOf(hashVals), alg.sum().size()));

	const ReportBlob *blob = 0;

	for (int h = 0; h < 4; ++h) {
		idx = (int)(hashVals[h] % (unsigned)theHash.capacity());
		if (stopAt(key, blob, idx))
			return blob;
	}

	// collision; use linear search
		
	for (int i = 0; i < theHash.capacity(); ++i, ++idx) {
		idx %= theHash.capacity();
		if (stopAt(key, blob, idx))
			return blob;
	}

	Assert(false); // no empty space in the hash
	return 0;
}

bool BlobIdx::stopAt(const Key &key, const ReportBlob *&blob, int idx) const {
	Assert(0 <= idx && idx <= theHash.count());

	if ((blob = theHash[idx])) {
		if (blob->key() == key)
			return true;

		blob = 0;
		return false; // collision
	}
	return true;
}

void BlobIdx::grow() {
	Array<const ReportBlob *> oldHash;
	oldHash = theHash;

	theHash.clear();
	theHash.resize(2*theHash.capacity() + 1);

	for (int i = 0; i < oldHash.count(); ++i) {
		if (oldHash[i])
			add(oldHash[i]);
	}
}
