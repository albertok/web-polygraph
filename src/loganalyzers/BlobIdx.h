
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_BLOBIDX_H
#define POLYGRAPH__LOGANALYZERS_BLOBIDX_H

class ReportBlob;
class String;

// key->blob index with fast search method
class BlobIdx {
	public:
		typedef String Key;

	public:
		BlobIdx(int aCapacity = 0);

		void add(const ReportBlob *blob);
		const ReportBlob *find(const Key &key) const;

	protected:
		const ReportBlob *find(const Key &key, int &idx) const;
		bool stopAt(const Key &key, const ReportBlob *&blob, int idx) const;
		void grow();

	protected:
		Array<const ReportBlob *> theHash;
		int theCount; // hash slots occupied
};

#endif
