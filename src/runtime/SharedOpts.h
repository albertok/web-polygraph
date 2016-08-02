
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_SHAREDOPTS_H
#define POLYGRAPH__RUNTIME_SHAREDOPTS_H

#include "base/opts.h"
#include "base/polyOpts.h"

// options shared by client and server sides
class SharedOpts: public OptGrp {
	public:
		SharedOpts(); // defaults

		virtual bool validate() const;
		virtual String ExpandMacros(const Opt &opt, const String &str) const;

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		HostTypeOpt theHostTypeOpt;

		IntOpt theVerbLevel;
		DumpFlagsOpt theDumpFlags;
		SizeOpt theDumpSize;
		NetAddrOpt theNotifAddr;
		NetAddrOpt theBeepDoormanListAt;
		NetAddrOpt theBeepDoormanSendTo;
		StrOpt theRunLabel;

		IntOpt theFDLimit;
		TwoIntOpt thePorts;

		StrOpt theCfgFileName;
		StrArrOpt theCfgDirs;

		StrOpt theConsFileName;
		StrOpt theLogFileName;
		SizeOpt theLogBufSize;
		StrOpt doStoreWorkSet;
		StrOpt doLoadWorkSet;
		StrOpt theSmplLogFileName;
		SizeOpt theSmplLogBufSize;

		BoolOpt acceptForeignMsgs;

		TimeOpt theStatCycleLen;

		FileScanOpt theFileScanner;
		IntOpt thePrioritySched;

		StrArrOpt theFakeHosts;
		BoolOpt deleteOldAliases;

		TimeOpt theIdleTout;

		IntOpt theLclRngSeed;
		IntOpt theGlbRngSeed;
		BoolOpt useUniqueWorld;
		IntOpt theWorkerId;

		TimeOpt theErrorTout;
};

extern SharedOpts TheOpts;

#endif
