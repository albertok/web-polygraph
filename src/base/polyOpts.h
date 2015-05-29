
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_POLYOPTS_H
#define POLYGRAPH__BASE_POLYOPTS_H

#include "base/opts.h"

class FileScanner;

// a collection of command line options that are very specific to Poly
// see include/opts.h for more generic options


// which system call to use for scanning for ready files
class FileScanOpt: public Opt {
	public:
		FileScanOpt(OptGrp *aGrp, const char *aName, const char *aDescr, FileScanner *def);

		virtual void report(ostream &) const;

		FileScanner *val() { return theVal; }
		operator void*() const { return theVal ? (void*)-1 : 0; }

	protected:
		virtual bool parse(const String &name, const String &val);

		FileScanner *theVal;
};


// various message dump flags
typedef enum { dumpNone = 0, dumpAny = ~0U,
	dumpReq = 1, dumpRep = 2, dumpErr = 3, dumpSum, dumpEmbedStats,
	dumpTypeCnt, // must be last
	dumpHdr = 1, dumpBody = 2 } DumpFlags;

class DumpFlagsOpt: public ListOpt {
	public:
		DumpFlagsOpt(OptGrp *aGrp, const char *aName, const char *aDescr);

		virtual void report(ostream &) const;

		bool operator ()(DumpFlags type, DumpFlags part = dumpAny) const { return (theFlags[type] & part) != 0; }

		void setAll(); // dump everything

		void setFlag(DumpFlags type, DumpFlags part);

	protected:
		virtual bool addItem(const String &item);
		const char *dumpPartStr(DumpFlags type) const;

	protected:
		unsigned int theFlags[dumpTypeCnt];
};


// generic interface, but poly-specific implementation
class HostTypeOpt: public Opt {
	public:
		HostTypeOpt(OptGrp *aGrp, const char *aName, const char *aDescr);

		virtual void report(ostream &) const;

	protected:
		virtual bool parse(const String &name, const String &val);
};

// generic interface, but poly-specific implementation
class VersionOpt: public Opt {
	public:
		VersionOpt(OptGrp *aGrp, const char *aName, const char *aDescr);

		virtual void report(ostream &) const;

	protected:
		virtual bool parse(const String &name, const String &val);
};

#endif
