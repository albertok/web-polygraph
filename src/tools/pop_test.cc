
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>

#include "xstd/h/iostream.h"
#include <fstream>
#include "xstd/h/iomanip.h"

#include "xstd/Rnd.h"
#include "base/RndPermut.h"
#include "xstd/Ring.h"
#include "tools/IntIntHash.h"
#include "base/CmdLine.h"
#include "base/opts.h"
#include "base/polyOpts.h"
#include "xstd/gadgets.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,      "help",             "list of options"),
			theVersOpt(this,      "version",          "package version info"),
			theOutFileName(this,  "out <file>",       "redirect console output", "-"),
			theCachabRatio(this,  "cachable <%>",     "portion of cachable replies", 0.80),
			thePublicRatio(this,  "public_interest <%>", "portion of URLs shared among all robots", 0.50),
			theRecurrRatio(this,  "recurrence <%>",   "probability of a re-visit to a URL", 0.55/0.80),
			theWorkSetSize(this,  "work_set_size <size>","working set size"),
			theCacheSize(this,    "cache_size <size>","cache size", BigSize::MB(100)),
			theObjSize(this,      "obj_size <size>",  "average object size", Size::KB(13)),
			theRobCount(this,     "robots <int>",     "total number of robots to simulate", 1),
			thePopModel(this,     "pop_model <str>",  "popularity model", "unif"),
			theSimLength(this,    "sim_length <int>", "total number of request to simulate", 50000000)
			{}

		virtual bool validate() const;

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theOutFileName;
		DblOpt theCachabRatio;
		DblOpt thePublicRatio;
		DblOpt theRecurrRatio;
		BigSizeOpt theWorkSetSize;
		BigSizeOpt theCacheSize;
		SizeOpt theObjSize;
		IntOpt theRobCount;
		StrOpt thePopModel;
		IntOpt theSimLength;
} TheOpts;

class Robot {
	public:
		Robot(int anOidOff);

		void step();

	protected:
		int genOid();

	protected:
		int theOidOff;
		int thePrivOidCnt;
};

class Cache {
	public:
		Cache(int aCapacity);

		double dhp() const { return Percent(theHitCount, theReqCount); }
		double intervalDhp();
		double utilp() const { return Percent(theSize, theCapacity); }
		bool full() const { return theSize >= theCapacity; }
		int capacity() const { return theCapacity; }
		int fill() const { return theFill; }
		int reqs() const { return theReqCount; }
		const IntIntHash &hash() const { return theIndex; }

		void noteObject(int oid);

	protected:
		void purge();

	protected:
		int theCapacity;
		int theSize;
		int theFill;

		Ring<int> theRepPolicy;
		IntIntHash theIndex;

		int theHitCount;
		int theReqCount;

		int theIntvlHitCount;
		int theIntvlReqCount;
};


static
struct Server {
	int theLastOid;
} TheServer;


typedef int (*PopModelPtr)(RndGen &rng, int lastOid);
static int PopModel(RndGen &rng, int lastOid, int wsCap);

static Array<Robot*> TheRobots;
static Cache *TheCache = 0;
static int TheOidLmt = -1; // limit for private and shared oids

static int TheTotlWorkSetCap = -1;
static int ThePrivWorkSubsetCap = -1;
static int TheShrdWorkSubsetCap = -1;

static PopModelPtr ThePopModel = 0;
static double ThePopModelParam = 0;


bool MyOpts::validate() const {
	if (theWorkSetSize <= 0)
		cerr << "working set size must be specified" << endl;
	else
		return true;
	return false;
}


/* Robot */

Robot::Robot(int anOidOff):	theOidOff(anOidOff), thePrivOidCnt(0) {
}

// mimics Client::genOid
int Robot::genOid() {
	static RndGen rng;
	const bool publicOid = rng() < TheOpts.thePublicRatio;
	const bool repeatOid = rng() < TheOpts.theRecurrRatio;

	if (publicOid) {
		if (repeatOid && TheServer.theLastOid > 0) {
			return PopModel(rng, TheServer.theLastOid, TheShrdWorkSubsetCap);
		}
		Assert(TheServer.theLastOid < TheOidLmt);
		return ++TheServer.theLastOid;
	}

	if (repeatOid && thePrivOidCnt > 0)
		return theOidOff + PopModel(rng, thePrivOidCnt, ThePrivWorkSubsetCap);

	Assert(thePrivOidCnt < TheOidLmt);
	return theOidOff + (++thePrivOidCnt);
}

void Robot::step() {
	const int oid = genOid();
	TheCache->noteObject(oid);
}


/* Cache */

Cache::Cache(int aCapacity): theCapacity(aCapacity), theSize(0), theFill(0),
	theRepPolicy(4*aCapacity), theIndex(aCapacity),
	theHitCount(0), theReqCount(0), theIntvlHitCount(0), theIntvlReqCount(0) {
}

void Cache::noteObject(int oid) {
	Assert(oid > 0);
	theReqCount++;
	theIntvlReqCount++;

	RndGen rng(LclPermut(oid));
	if (rng() > TheOpts.theCachabRatio)
		return; // uncachable object

	IntIntHash::Loc loc;
	if (theIndex.find(oid, loc)) {
		theHitCount++;
		theIntvlHitCount++;
		theIndex[loc]++;
	} else {
		const bool wasFull = full();
		theIndex.addAt(loc, oid, 1);
		theSize++;
		theFill++;

		if (wasFull)
			purge();
	}

	Assert(!theRepPolicy.full());
	theRepPolicy.enqueue(oid);
}

void Cache::purge() {
	Assert(theSize > 0);
	while (1) {
		Assert(!theRepPolicy.empty());
		const int oid = theRepPolicy.dequeue();
		IntIntHash::Loc loc;
		Assert(theIndex.find(oid, loc));
		int &ttl = theIndex[loc];
		Assert(ttl > 0);
		if (--ttl == 0) {
			theIndex.delAt(loc);
			break;
		}
	}
	theSize--;
}

double Cache::intervalDhp() {
	const double res = Percent(theIntvlHitCount, theIntvlReqCount);
	theIntvlHitCount = theIntvlReqCount = 0;
	return res;
}

static
void configureLogs(int prec) {
	if (TheOpts.theOutFileName && TheOpts.theOutFileName != "-")
		redirectOutput(TheOpts.theOutFileName.cstr());

	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);
}


/* Pop Models */

static
int UnifPopModel(RndGen &rng, int lastOid) {
	return 1 + rng(0, lastOid);
}


int Zipf(double alpha, RndGen &rng, int lastOid) {
	const double rn = rng();
	return (int)pow(lastOid+1, pow(rn,alpha));
}

static
int ZipfPopModel(RndGen &rng, int lastOid) {
	return 1 + lastOid - Zipf(ThePopModelParam, rng, lastOid);
}

inline double logd(double x) { return log(x); }

static
int ZipdPopModel(RndGen &rng, int lastOid) {
	if (lastOid == 1 || ThePopModelParam >= 1)
		return lastOid;
	const double alpha = logd(logd(2)/logd(lastOid+1)) / logd(ThePopModelParam);
	return 1 + lastOid - Zipf(alpha, rng, lastOid);
}

static
int PopModel(RndGen &rng, int lastOid, int wsCap) {
	const int offset = lastOid > wsCap ? lastOid-wsCap : 0;
	return offset + ThePopModel(rng, lastOid-offset);
}

// set some general stuff and
// propogate cmd line options to corresponding objects
static
void configure() {
	configureLogs(2);

	// this is total work set size
	TheTotlWorkSetCap = 1 + (int) (TheOpts.theWorkSetSize / BigSize(TheOpts.theObjSize));

	// compute private work subsets for robots and "shared" subset
	ThePrivWorkSubsetCap = 1 + (int)((1-TheOpts.thePublicRatio)*TheTotlWorkSetCap/(int)TheOpts.theRobCount);
	TheShrdWorkSubsetCap = 1 + (int)(TheOpts.thePublicRatio*TheTotlWorkSetCap);
	
	// note: we do not adjust subsets for uncachable objects because
	// robots have no idea and do not care what is cachable

	String pmName = TheOpts.thePopModel;
	if (const char *p = pmName.chr(':')) {
		isNum(p+1, ThePopModelParam);
		pmName = pmName(0, p-pmName.cstr());
	}

	if (TheOpts.thePopModel == "unif") {
		ThePopModel = &UnifPopModel;
	} else 
	if (pmName == "zipf") {
		ThePopModel = &ZipfPopModel;
		if (ThePopModelParam <= 0)
			ThePopModelParam = 1;
	} else
	if (pmName == "zipd") {
		ThePopModel = &ZipdPopModel;
		if (ThePopModelParam <= 0)
			ThePopModelParam = 0.5/100;
	} else {
		cerr << "unknown popularity model `" << TheOpts.thePopModel << "'" << endl;
		exit(-1);
	}

	const int objInCache = 1 + (int) (TheOpts.theCacheSize / BigSize(TheOpts.theObjSize));
	TheCache = new Cache(objInCache);

	// oid limits
	TheOidLmt = INT_MAX / (1 + (int)TheOpts.theRobCount);

	for (int i = 0; i < TheOpts.theRobCount; ++i)
		TheRobots.append(new Robot((i+1)*TheOidLmt));
}

static
void report() {
	static int repCnt = 0;
	if (!repCnt++) {
		cout << '#'
			<< ' ' << setw(8) << "reqs"
			<< ' ' << setw(8) << "fill#"
			<< ' ' << setw(8) << "fill%"
			<< ' ' << setw(6) << "DHRi"
			<< ' ' << setw(6) << "DHR"
			<< endl;
	}

	cout << 'i'
		<< ' ' << setw(8) << TheCache->reqs()
		<< ' ' << setw(8) << TheCache->fill()
		<< ' ' << setw(8) << (int)Percent(TheCache->fill(), TheCache->capacity())
		<< ' ' << setw(6) << TheCache->intervalDhp()
		<< ' ' << setw(6) << TheCache->dhp()
		<< endl;
}

static
void run() {
	static RndGen rng;

	bool full = TheCache->full();
	if (!full)
		cout << "# filling the cache..." << endl;

	const int repCycle = Max(1, TheOpts.theSimLength/1000, TheCache->capacity() / 20);
	for (int i = TheOpts.theSimLength; i; --i) {
		// select a random robot
		Robot *robot = TheRobots[rng(0, TheRobots.count())];
		robot->step();

		if ((full && abs(i) % repCycle == 1) || (!full && TheCache->full()))
			report();

		full = TheCache->full();
	}
}

int main(int argc, char *argv[]) {
	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	configure();
	cmd.report(cout);

	cout << "# " << TheOpts.theCacheSize << " cache fits " << TheCache->capacity() << " objects" << endl;
	cout << "# " << "working set is " << TheOpts.theWorkSetSize << " or " << TheTotlWorkSetCap << " objects" << endl;
	cout << "# " << "working set split: " << TheRobots.count() << " * " << ThePrivWorkSubsetCap << " + " << TheShrdWorkSubsetCap << " objects" << endl;
	cout << "# " << "server and robot world limit is " << TheOidLmt << " oids each" << endl;

	run();

	return 0;
}
