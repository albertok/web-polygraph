
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/string.h"

#include "xstd/Select.h"
#include "xstd/Poll.h"
#include "xstd/Epoll.h"
#include "xstd/rndDistrs.h"
#include "base/opts.h"
#include "base/polyOpts.h"
#include "base/polyVersion.h"
#include "base/PatchRegistry.h"
#include "xstd/gadgets.h"


/* FileScanOpt */

FileScanOpt::FileScanOpt(OptGrp *aGrp, const char *aName, const char *aDescr, FileScanner *def):
	Opt(aGrp, aName, aDescr), theVal(def) {
}

bool FileScanOpt::parse(const String &name, const String &val) {

	delete theVal; theVal = 0;

	const String v = val;
	if (v == "select")
		theVal = new Select();
	else
	if (v == "poll")
		theVal = new Poll();
	else
	if (v == "epoll")
		theVal = new Epoll();
	else
		cerr << "unknown file scanner name (use `epoll', `poll' or `select') for the `" << name << "' option; got: " << v << endl;

	return theVal != 0;
}

void FileScanOpt::report(ostream &os) const {
	os << (theVal ? theVal->name() : "<none>");
}


/* DumpFlagsOpt */

DumpFlagsOpt::DumpFlagsOpt(OptGrp *aGrp, const char *aName, const char *aDescr):
	ListOpt(aGrp, aName, aDescr) {
	memset(theFlags, 0, sizeof(theFlags));
}

void DumpFlagsOpt::setAll() {
	for (unsigned int i = 0; i < dumpTypeCnt; ++i)
		theFlags[i] = dumpAny;
}

bool DumpFlagsOpt::addItem(const String &item) {
	// which part of the message to dump
	DumpFlags part = dumpAny;
	if (item.str("hdr"))
		part = dumpHdr;
	else
	if (item.str("body"))
		part = dumpBody;

	// what kind of messages to dump
	if (item.str("req"))
		setFlag(dumpReq, part);
	else
	if (item.str("rep"))
		setFlag(dumpRep, part);
	else
	if (item.str("err"))
		setFlag(dumpErr, part);
	else
	if (item.str("sum"))
		setFlag(dumpSum, part);
	else
	if (item.str("embed_stats"))
		setFlag(dumpEmbedStats, part);
	else
	if (part != dumpAny || item == "All") {
		setFlag(dumpReq, part);
		setFlag(dumpRep, part);
		setFlag(dumpErr, part);
		if (item == "All")
			setFlag(dumpSum, part);
		setFlag(dumpEmbedStats, part);
	} else {
		cerr << "unknown flag `" << item << "' for the `" << name() << "' option" << endl;
		return false;
	}

	return true;
}

void DumpFlagsOpt::setFlag(DumpFlags type, DumpFlags part) {
	theFlags[type] |= part;
	theFlags[0] |= part;
}

const char *DumpFlagsOpt::dumpPartStr(DumpFlags type) const {
	if (theFlags[type] == dumpHdr)
		return "-hdr";
	if (theFlags[type] == dumpBody)
		return "-body";
	return "";
}

void DumpFlagsOpt::report(ostream &os) const {
	if (!theFlags[0]) {
		os << "<none>";
		return;
	}
		
	if (theFlags[dumpReq])
		os << "req," << dumpPartStr(dumpReq);
	if (theFlags[dumpRep])
		os << "rep," << dumpPartStr(dumpRep);
	if (theFlags[dumpErr])
		os << "err," << dumpPartStr(dumpErr);
	if (theFlags[dumpSum])
		os << "sum" << dumpPartStr(dumpSum);
	if (theFlags[dumpEmbedStats])
		os << "embed_stats" << dumpPartStr(dumpEmbedStats);
}


/* HostTypeOpt */

HostTypeOpt::HostTypeOpt(OptGrp *aGrp, const char *aName, const char *aDescr):
	Opt(aGrp, aName, aDescr) {
}

bool HostTypeOpt::parse(const String &, const String &) {
	Assert(theCmdLine);
	cerr
		<< "version " << PolyVersion()
		<< " built on " << CONFIG_HOST_TYPE
		<< " at " << PolyCompTime() << ' ' << PolyCompDate()
		<< endl;
	exit(0);
	return false;
}

void HostTypeOpt::report(ostream &os) const {
	os << CONFIG_HOST_TYPE;
}


/* VersionOpt */

VersionOpt::VersionOpt(OptGrp *aGrp, const char *aName, const char *aDescr):
	Opt(aGrp, aName, aDescr) {
}

bool VersionOpt::parse(const String &, const String &) {
	Assert(theCmdLine);
	cerr << PolyLetterhd() << endl;
	ReportPatches(cerr);
	exit(0);
	return false;
}

void VersionOpt::report(ostream &os) const {
	os << PolyVersion();
	if (const int count = CountPatches())
		os << " with " << count << " patch(es)";
}
