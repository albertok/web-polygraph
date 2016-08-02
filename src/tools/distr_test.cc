
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>

#include "xstd/h/iostream.h"
#include <fstream>
#include "xstd/h/iomanip.h"

#include "base/CmdLine.h"
#include "base/ObjId.h"
#include "base/RndPermut.h"
#include "base/opts.h"
#include "base/polyOpts.h"
#include "base/histograms.h"
#include "xstd/gadgets.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,      "help",           "list of options"),
			theVersOpt(this,      "version",        "package version info"),
			theOutFileName(this,  "out <file>",     "redirect console output", "-"),
			theDistr(this,  "distr <numDistr>",     "distribution to test"),
			theStep(this,  "hist_step <%>",      "bin `size' in histogram output", 0.05/100),
			theCount(this,  "count <int>",      "number of trials", 100000),
			useOidSeeding(this, "oid_seeding <bool>", "oid-based r.n.g. seeding", false),
			theRngSeed(this, "rng_seed <int>", "r.n.g. seed", 1),
			theLclRngSeed(this, "local_rng_seed <int>", "per-process r.n.g. seed", 1),
			theGlbRngSeed(this, "global_rng_seed <int>", "per-test r.n.g. seed", 1),
			theWorldCount(this, "world_count <int>", "number of simulated worlds", 1),
			useUniqueWorld(this, "unique_world <bool>", "use URL set that is unique across runs", true)
			{}

		virtual bool validate() const;

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theOutFileName;
		DistrOpt theDistr;
		DblOpt theStep;
		IntOpt theCount;
		BoolOpt useOidSeeding;
		IntOpt theRngSeed;
		IntOpt theLclRngSeed;
		IntOpt theGlbRngSeed;
		IntOpt theWorldCount;
		BoolOpt useUniqueWorld;
} TheOpts;

bool MyOpts::validate() const {
	if (!theDistr)
		cerr << "must specify the distribution to test" << endl;
	else
		return true;
	return false;
}

static
void configureLogs(int prec) {
	if (TheOpts.theOutFileName && TheOpts.theOutFileName != "-")
		redirectOutput(TheOpts.theOutFileName.cstr());

	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);
}

// set some general stuff and
// propogate cmd line options to corresponding objects
static
void configure() {
	configureLogs(2);

	if (TheOpts.useOidSeeding) {
		if (TheOpts.theRngSeed.wasSet()) {
			cerr << "error: rng_seed option can not be used with "
				"oid-based seeding" << endl << xexit;
		}

		// set random seeds
		GlbPermut().reseed(TheOpts.theGlbRngSeed);
		LclPermut().reseed(TheOpts.theLclRngSeed);

		// use the seed as uid "space" index for non-unique worlds
		if (!TheOpts.useUniqueWorld)
			UniqId::Space(TheOpts.theLclRngSeed);
	} else {
		if (TheOpts.theLclRngSeed.wasSet() ||
			TheOpts.theGlbRngSeed.wasSet() ||
			TheOpts.theWorldCount.wasSet() ||
			TheOpts.useUniqueWorld.wasSet()) {
			cerr << "error: local_rng_seed, global_rng_seed, "
				"world_count, and unique_world options can "
				"only be used with oid-based seeding"
				<< endl << xexit;
		}

		TheOpts.theDistr.condDistr()->rndGen()->seed(TheOpts.theRngSeed);
	}
	if (TheOpts.theCount % TheOpts.theWorldCount) {
		cerr << "warning: trials count is not divisible by worlds count"
			<< endl;
	}
}

static
void reportOption(const Opt& opt) {
	static const String prefix = "config.";

	const int plen = opt.name().len() + 1;
	cout << prefix << opt.name() << ':' << setw(20-plen) << "";
	opt.report(cout);
	cout << endl;
}

static
void reportConfiguration() {
	reportOption(TheOpts.theOutFileName);
	reportOption(TheOpts.theDistr);
	reportOption(TheOpts.theStep);
	reportOption(TheOpts.theCount);
	reportOption(TheOpts.useOidSeeding);
	if (TheOpts.useOidSeeding) {
		reportOption(TheOpts.theLclRngSeed);
		reportOption(TheOpts.theGlbRngSeed);
		reportOption(TheOpts.theWorldCount);
		reportOption(TheOpts.useUniqueWorld);
	} else
		reportOption(TheOpts.theRngSeed);
	cout << endl;
}

static
void testOneWorld(RndDistr *const distr, Histogram *const hist) {
	const int count = TheOpts.theCount / TheOpts.theWorldCount;
	ObjId oid;
	oid.world(UniqId::Create());

	for (int i = 0; i < count; ++i) {
		if (TheOpts.useOidSeeding) {
			oid.name(i + 1);
			const int seed = GlbPermut(oid.hash(), rndRepSize);
			distr->rndGen()->seed(seed);
		}
		hist->record((int)Min(distr->trial(), (double)INT_MAX));
	}
}

static
void test() {
	RndDistr *const distr = TheOpts.theDistr.condDistr();
	Histogram *const hist = new Log2Hist();
	for (int i = 0; i < TheOpts.theWorldCount; ++i)
		testOneWorld(distr, hist);
	hist->report(TheOpts.theStep, cout);
	cout << endl << hist->stats();
	delete hist;
}

int main(int argc, char *argv[]) {
	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate() || !TheOpts.validate())
		return -1;

	configure();
	reportConfiguration();

	test();

	return 0;
}
