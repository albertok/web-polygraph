
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_PERSISTWORKSETMGR_H
#define POLYGRAPH__RUNTIME_PERSISTWORKSETMGR_H

#include "base/UniqId.h"

class String;
class IBStream;
class OBStream;

// persistent working set
class PersistWorkSetMgr {
	public:
		PersistWorkSetMgr();
		~PersistWorkSetMgr();

		void configure();

		const UniqId &id() const;
		int version() const;

		void openInput(const String &anInFname);
		void openOutput(const String &anOutFname);

		void loadSeeds();
		void storeSeeds();

		void loadUniverses();
		void storeUniverses();

		IBStream *loadSideState();  // may be null
		OBStream *storeSideState(); // may be null

		void checkInput();
		void checkOutput();

		void closeInput();
		void closeOutput();
		void close();

	protected:
		void loadHeader();
		void storeHeader();

		void loadMagic();
		void storeMagic();

		void loadTag(int expectedTag);
		void storeTag(int expectedTag);

	private:
		IBStream *theInStream;
		OBStream *theOutStream;
		UniqId theId;
		int theVersion;
};

extern PersistWorkSetMgr ThePersistWorkSetMgr;

#endif
