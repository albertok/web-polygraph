
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CONTENTDBASE_H
#define POLYGRAPH__CSM_CONTENTDBASE_H

#include "xstd/String.h"
#include "xstd/Array.h"
#include "base/ObjId.h"

class OBStream;
class IBStream;
class WrBuf;
class InjectIter;
class EmbedContMdl;
class CdbEntry;
class CdbEntryPrnOpt;

// contains compiled information about content of Web objects
// can be used to simulate realistic content

class ContentDbase {
	public:
		static CdbEntry *CreateEntry(int type);

	public:
		ContentDbase();
		~ContentDbase();

		const String &name() const { return theName; }
		int count() const; // number of entries
		bool hasLinkOrPage() const; // at least one cdbeLink or cdbePage entry
		double entrySizeMean() const; // mean entry size
		const CdbEntry *entry(int pos) const { return theEntries[pos]; }

		void add(CdbEntry *e);

		void load(IBStream &is);
		void store(OBStream &os) const;

		ostream &print(ostream &os) const;

	protected:
		Array<CdbEntry*> theEntries;

		mutable String theName; // copied from the last BStream
};


#endif
