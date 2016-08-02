
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>
#include "xstd/h/fcntl.h"

#include "xstd/String.h"
#include "base/BStream.h"
#include "runtime/LogComment.h"
#include "runtime/ObjUniverse.h"
#include "runtime/HostMap.h"
#include "runtime/Viservs.h"
#include "runtime/SharedOpts.h"
#include "runtime/PersistWorkSetMgr.h"

PersistWorkSetMgr ThePersistWorkSetMgr;

enum PersistWorkSetMagic { pwsMagic1 = 0x506f6c79, pwsMagic2 = 0x57536574 };
const int PersistWorkSetFormatVersion = 1;
enum PersistWorkSetTags { pwsStart, pwsSeeds, pwsUniverses, pwsSideState };


PersistWorkSetMgr::PersistWorkSetMgr(): theInStream(0), theOutStream(0),
	theVersion(0) {
}

PersistWorkSetMgr::~PersistWorkSetMgr() {
	close();
}

void PersistWorkSetMgr::configure() {
	if (!theId) // create a unique ID if it was not loaded from disk
		theId.create();
}

const UniqId &PersistWorkSetMgr::id() const {
	return theId;
}

int PersistWorkSetMgr::version() const {
	return theVersion;
}

void PersistWorkSetMgr::openInput(const String &inFname) {
	Assert(!theInStream);
	if (inFname) {
		istream *is = new ifstream(inFname.cstr(),
			ios::binary|ios::in);
		theInStream = new IBStream;
		theInStream->configure(is, inFname);
		loadHeader();
	}
}

void PersistWorkSetMgr::openOutput(const String &outFname) {
	Assert(!theOutStream);
	if (outFname) {
		ostream *os = new ofstream(outFname.cstr(), 
			ios::binary|ios::out|ios::trunc);
		theOutStream = new OBStream;
		theOutStream->configure(os, outFname);

		theVersion++;
		storeHeader();
	}
}

void PersistWorkSetMgr::close() {
	closeInput();
	closeOutput();
}

void PersistWorkSetMgr::closeInput() {
	if (theInStream) {
		istream *is = theInStream->stream();
		delete theInStream;
		theInStream = 0;
		delete is;
	}
}

void PersistWorkSetMgr::closeOutput() {
	if (theOutStream) {
		ostream *os = theOutStream->stream();
		delete theOutStream;
		theOutStream = 0;
		delete os;
	}
}

void PersistWorkSetMgr::loadHeader() {
	if (!theInStream)
		return;

	checkInput();
	loadMagic();
	*theInStream >> theId >> theVersion;
	checkInput();

	if (!theId || theVersion <= 0) {
		cerr << here << "malformed persistent working set file: " <<
			theInStream->name() << ", stopped" << endl;
		exit(2);
	}
}

void PersistWorkSetMgr::storeHeader() {
	if (!theOutStream)
		return;

	checkOutput();
	storeMagic();
	*theOutStream << theId << theVersion;
	checkOutput();
}

void PersistWorkSetMgr::loadMagic() {
	if (!theInStream)
		return;

	const bool res = 
		theInStream->geti() == pwsMagic1 &&
		theInStream->geti() == pwsMagic2;
	checkInput();

	if (!res) {
		cerr << here << "malformed persistent working set file: " <<
			theInStream->name() << ", stopped" << endl;
		exit(2);
	}

	const int formatVersion = theInStream->geti();
	if (formatVersion != PersistWorkSetFormatVersion) {
		cerr << here << "unsupported persistent working set file: " <<
			theInStream->name() << endl <<
			"file version: " << formatVersion << endl <<
			"supported version: " << PersistWorkSetFormatVersion <<
			endl;
		exit(2);
	}
}

void PersistWorkSetMgr::storeMagic() {
	if (!theOutStream)
		return;
	*theOutStream << pwsMagic1 << pwsMagic2 << PersistWorkSetFormatVersion;
	checkOutput();
}

void PersistWorkSetMgr::loadSeeds() {
	if (!theInStream)
		return;

	loadTag(pwsSeeds);

	const int lclSeed = theInStream->geti();
	const int glbSeed = theInStream->geti();
	checkInput();

	if (lclSeed <= 0 || glbSeed <= 0) {
		cerr << here << "error loading persistent working set from  " <<
			theInStream->name() << endl;
		exit(2);
	}

	TheOpts.theLclRngSeed.set(lclSeed);
	TheOpts.theGlbRngSeed.set(glbSeed);
}

void PersistWorkSetMgr::storeSeeds() {
	if (!theOutStream)
		return;

	storeTag(pwsSeeds);

	*theOutStream << (int)TheOpts.theLclRngSeed << (int)TheOpts.theGlbRngSeed;
	checkOutput();
}

void PersistWorkSetMgr::loadUniverses() {
	if (!theInStream)
		return;

	const int universeCount = theInStream->geti();
	checkInput();
	for (int i = 0; i < universeCount; ++i) {
		NetAddr server;
		ObjUniverse *universe = new ObjUniverse;
		*theInStream >> server >> *universe;
		checkInput();
		if (HostCfg *host = TheHostMap->find(server)) {
			ObjUniverse::Put(host, universe);
		} else {
			Comment << "error: visible server " << server << " in the " <<
				"working set being loaded from " << theInStream->name() <<
				" is not on the current configuration, skipping" << endc;
			delete universe;
		}
	}

	// here we cannot check that all current viservers have stored universes
	//Universe::DumpSlices(cerr << here);
}

void PersistWorkSetMgr::storeUniverses() {
	if (!theOutStream)
		return;

	*theOutStream << ObjUniverse::Count();
	checkOutput();
	for (ViservIterator i; !i.atEnd(); ++i)
		*theOutStream << i.host()->theAddr << *i.universe();
	checkOutput();

	//Universe::DumpSlices(cerr << here);
}

IBStream *PersistWorkSetMgr::loadSideState() {
	if (!theInStream)
		return 0;

	checkInput();
	return theInStream;
}

OBStream *PersistWorkSetMgr::storeSideState() {
	if (!theOutStream)
		return 0;

	checkOutput();
	return theOutStream;
}

void PersistWorkSetMgr::loadTag(int expectedTag) {
	const int tag = theInStream->geti();
	checkInput();

	if (tag != expectedTag) {
		cerr << here << "invalid persistent working set format in " <<
			theInStream->name() << ", " <<
			"stopped (" << tag << '/' << expectedTag << ")." << endl;
		exit(2);
	}
}

void PersistWorkSetMgr::storeTag(int tag) {
	*theOutStream << tag;
	checkOutput();
}

void PersistWorkSetMgr::checkInput() {
	if (theInStream && !theInStream->good()) {
		cerr << here << "read error while loading persistent working set " <<
			"from " << theInStream->name();
		if (const Error err = Error::Last())
			cerr << ": " << err;
		cerr << endl;
		exit(2);
	}
}

void PersistWorkSetMgr::checkOutput() {
	if (theOutStream && !theOutStream->good()) {
		cerr << here << "write error while storing persistent working set " <<
			"in " << theInStream->name();
		if (const Error err = Error::Last())
			cerr << ": " << err;
		cerr << endl;
		exit(2);
	}
}
