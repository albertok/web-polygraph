
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Select.h"
#include "xstd/Poll.h"
#include "xstd/Epoll.h"
#include "runtime/SharedOpts.h"
#include "runtime/globals.h"
#include "base/macros.h"
#include "base/AnyToString.h"

SharedOpts TheOpts;

SharedOpts::SharedOpts():
	theHelpOpt(this,      "help",               "list of options"),
	theVersOpt(this,      "version",            "package version info"),
	theHostTypeOpt(this,  "host_type",          "package build environment info"),

	theVerbLevel(this,    "verb_lvl <int>",     "console verbosity level", 0),
	theDumpFlags(this,    "dump <list>",        "dump request/replies/errors headers/bodies to console"),
	theDumpSize(this,     "dump_size <size>",   "limit size of individual msg dump", Size::KB(1)),
	theNotifAddr(this,    "notify <addr>",      "send notifications to a [remote] monitoring program", NetAddr()),
	theBeepDoormanListAt(this,"doorman_listen_at <addr>","where to listen for (and respond to) meta-information requests", NetAddr()),
	theBeepDoormanSendTo(this,"doorman_send_to <addr>","where to forward meta-information", NetAddr()),
	theRunLabel(this,     "label <str>",        "run label"),

	theFDLimit(this,      "fd_limit <int>",     "decrease default fd limit"),
	thePorts(this,        "ports <port_range>", "TCP port range for explicit binding"),

	theCfgFileName(this,  "config <file>",		"configuration file name"),
	theCfgDirs(this,      "cfg_dirs <list>",    "directories to search for cfg files"),

	theConsFileName(this, "console <file>",     "redirect console output", "-"),
	theLogFileName(this,  "log <file>",         "detailed binary log file"),
	theLogBufSize(this,   "log_buf_size <size>","buffer size for binary log"),
	doStoreWorkSet(this,  "store_working_set <file>", "store working set"),
	doLoadWorkSet(this,   "load_working_set <file>", "load working set"),
	theSmplLogFileName(this,"sample_log <file>",  "log file for samples"),
	theSmplLogBufSize(this, "sample_log_buf_size <size>","buffer size for sample log"),

	acceptForeignMsgs(this,"accept_foreign_msgs", "accept foreign requests or responses", false),

	theStatCycleLen(this, "stats_cycle <time>", "statistics cycle length", Time::Sec(5)),

	theFileScanner(this,  "file_scan <epoll|poll|select>", "system call to use for scanning ready files", new PG_PREFFERED_FILE_SCANNER),
	thePrioritySched(this,"priority_sched <int>", "priority level for urgent socket operations", 5),

	theFakeHosts(this,    "fake_hosts <list>",  "use given addresses, do not lookup real interfaces"),
	deleteOldAliases(this, "delete_old_addrs <bool>", "delete old IP aliases before adding new ones", true),

	theIdleTout(this,     "idle_tout <time>",   "exit after inactivity timeout"),

	theLclRngSeed(this,   "local_rng_seed <int>", "per-process r.n.g. seed", 1),
	theGlbRngSeed(this,   "global_rng_seed <int>","per-test r.n.g. seed", 1),
	useUniqueWorld(this,  "unique_world <bool>","use URL set that is unique across runs", true),
	theWorkerId(this,     "worker <int>", "SMP worker ID", 0),
	theErrorTout(this,    "hushed_error_tout <time>", "hush frequent error reporting for that long", Time::Sec(60))
{
	theDumpFlags.setFlag(dumpErr, dumpAny);
}

bool SharedOpts::validate() const {
	if (theWorkerId.wasSet() && theWorkerId <= 0)
		cerr << ThePrgName << ": SMP worker ID must be positive; got: " << theWorkerId << endl;
	else
	if (theLclRngSeed <= 0)
		cerr << ThePrgName << ": local r.n.g. seed must be positive; got: " << theLclRngSeed << endl;
	else
	if (theGlbRngSeed <= 0)
		cerr << ThePrgName << ": global r.n.g. seed must be positive; got: " << theGlbRngSeed << endl;
	else
	if (theStatCycleLen <= 0)
		cerr << ThePrgName << ": the stats cycle length must be positive; got: " << theStatCycleLen << endl;
	else
	if (!theFileScanner)
		cerr << ThePrgName << ": the file scanner (e.g. `select', `poll' or `epoll`) must be specified" << endl;
	else
	if (thePrioritySched <= 0)
		cerr << ThePrgName << ": priority scheduling level must be positive; got: " << thePrioritySched << endl;
	else
	if (!theCfgFileName)
		cerr << ThePrgName << ": configuration file must be specified (see --config option)" << endl;
	else
		return OptGrp::validate();
	return false;
}

String SharedOpts::ExpandMacros(const Opt &opt, const String &str) const {
	if (opt.name() != "worker")
		return ExpandMacro(str, "%worker", AnyToString(theWorkerId));
	return str;
}
