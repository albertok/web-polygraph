
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Epoll.h"
#include "xstd/Poll.h"
#include "xstd/Select.h"
#include "probe/ProbeOpts.h"

ProbeOpts TheProbeOpts;

ProbeOpts::ProbeOpts():
	theHelpOpt(this,  "help",                "list of options"),
	theVersOpt(this,  "version",             "package version info"),
	theDuration(this, "duration <time>",     "test duration", Time::Sec(60)),
	theContTout(this, "contact_tout <time>", "agent contact attempt timeout", Time::Sec(1)),
	theStatExchTout(this, "stats_exchange_tout <time>", "time to exchange stats with remote agents", Time::Sec(60)),
	theCltHosts(this, "clients <str>",       "ip(s) or ip range(s) of client agents"),
	theSrvHosts(this, "servers <str>",       "ip(s) or ip range(s) of server agents"),
	theConsFileName(this, "console <file>",   "redirect console output", "-"),
	theFileScanner(this,  "file_scan <epoll|poll|select>", "system call to use for scanning ready fds", new PG_PREFFERED_FILE_SCANNER) {
}

ostream &ProbeOpts::printAnonym(ostream &os) const {
	return os << "[clt_hosts srv_hosts:port]";
}

bool ProbeOpts::parseAnonym(const Array<const char *> &opts) {
	if (!opts.count())
		return true;

	if (opts.count() != 2) {
		cerr << "malformed anonymous options, use --help" << endl;
		return false;
	}

	return 
		theCltHosts.set(theCltHosts.name(), opts[0]) &&
		theSrvHosts.set(theSrvHosts.name(), opts[1]);
}

bool ProbeOpts::validate() const {
	if (!theSrvHosts)
		cerr << "no server addresses specified" << endl;
	else
		return true;
	return false;
}

