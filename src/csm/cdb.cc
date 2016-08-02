
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include <fstream>
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"
#include "xstd/h/os_std.h"
#include "xstd/h/sys/stat.h"
#include "xstd/h/sys/types.h"

#include "xstd/gadgets.h"
#include "base/BStream.h"
#include "base/AnyToString.h"
#include "base/CmdLine.h"
#include "base/macros.h"
#include "base/opts.h"
#include "base/polyOpts.h"
#include "csm/ContentDbase.h"
#include "csm/cdbEntries.h"
#include "csm/cdbBuilders.h"
#include "csm/XmlParser.h"

static String ThePrgName = "";
static enum { fmtVerbatim, fmtMarkup, fmtLinkOnly } TheFormat = fmtMarkup;
static int TheTotalLinkCount = 0;

class MyOpts: public OptGrp {
	public:
		MyOpts():
			theAddDb(this, "add <file>", "add file(s) contents to the database"),
			theDumpDb(this, "dump <file>", "dump verbatim and linkonly objects from database, one object per file"),
			theShowDb(this, "show <file>", "print database contents to stdout"),
			theAsPath(this, "as <path>", "path for dumped object files, default './cdb${name}'", "cdb${name}"),
			theFormat(this, "format <markup|linkonly|verbatim>", "split each file into markup tags, adjust links, or add as is, default markup", "markup"),
			theHelpOpt(this, "help", "list of options"),
		    	theVersOpt(this, "version", "package version info")
			{}

		virtual ostream &printAnonym(ostream &os) const;
		virtual bool parseAnonym(const Array<const char *> &opts);
		virtual bool canParseAnonym() const { return true; }

		virtual bool validate() const;

	public:
		StrOpt theAddDb;
		StrOpt theDumpDb;
		StrOpt theShowDb;
		StrOpt theAsPath;
		StrOpt theFormat;
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		PtrArray<String*> theFiles;
};

static MyOpts TheOpts;

/* MyOpt */

ostream &MyOpts::printAnonym(ostream &os) const {
	return os << "[filename ...]";
}

bool MyOpts::parseAnonym(const Array<const char *> &opts) {
	theFiles.stretch(opts.count());
	for (int i = 0 ; i < opts.count(); ++i) {
		theFiles.append(new String(opts[i]));
	}
	return true;
}

bool MyOpts::validate() const {
	const StrOpt *const commands[] = { &theAddDb, &theDumpDb, &theShowDb };
	unsigned int commandsCount = 0;
	for (unsigned int i = 0; i < sizeof(commands) / sizeof(*commands); ++i) {
		if (*commands[i])
			++commandsCount;
	}
	if (!commandsCount) {
		cerr << "one of --add, --show or --dump option must be used" << endl;
		return false;
	} else 	if (commandsCount > 1) {
		cerr << "only one of --add, --show or --dump option can be used" << endl;
		return false;
	}

	if (theFormat.wasSet() && !theAddDb) {
		cerr << "option --format is allowed with --add only" << endl;
		return false;
	}
	if (theAsPath.wasSet() && !theDumpDb) {
		cerr << "option --as is allowed with --dump only" << endl;
		return false;
	}

	if (theFormat == "markup")
		TheFormat = fmtMarkup;
	else
	if (theFormat == "linkonly")
		TheFormat = fmtLinkOnly;
	else
	if (theFormat == "verbatim")
		TheFormat = fmtVerbatim;
	else {
		cerr << "unknown format '" << theFormat << "', expected 'markup', 'linkonly' or 'verbatim'" << endl;
		return false;
	}

	if (!theAsPath.str("${name}")) {
		cerr << "--as value must contain ${name} macro, e.g. './cdb${name}'" << endl;
		return false;
	}

	return true;
}

static
bool readCont(ContentDbase *cdb, const String &fname) {
	CdbBuilder::TheLinkCount = 0;

	const String hname = fname == "-" ? String("stdin") : fname;
	clog << ThePrgName << ": adding "
		<< (TheFormat == fmtMarkup ? "objects from " : "entire ")
		<< "'" << hname << "' ... ";

	const int savedCount = cdb->count();
	CdbBuilder *b = TheFormat == fmtMarkup ? (CdbBuilder*) new MarkupParser :
		(TheFormat == fmtLinkOnly ? (CdbBuilder*) new LinkOnlyParser :
		(CdbBuilder*) new VerbatimParser);

	istream *is = fname == "-" ? 
		(istream*)&cin : (istream*)new ifstream(fname.cstr());
	b->db(cdb);

	unsigned long bufsize = 128*1024;
	unsigned long bufpos = 0;
	char * buf = new char[bufsize];

	while(is->good())
	{
	    if (bufpos == bufsize)
	    {
		char * temp = new char[bufsize*2];
		memcpy(temp, buf, bufsize);
		bufsize += bufsize;
		delete buf;
		buf = temp;
	    }
	    is->read(buf + bufpos, bufsize - bufpos);
	    bufpos += is->gcount();
	}

	b->configure(fname, buf, buf + bufpos);
	const bool res = b->parse();
	if (is != &cin)
		delete is;

	delete b;
	delete buf;

	if (res) {
		TheTotalLinkCount += CdbBuilder::TheLinkCount;
		clog << "\t " << setw(6) << (cdb->count() - savedCount) <<
			" object(s) and " << CdbBuilder::TheLinkCount << " link(s)";
	}
	clog << endl;
	return res;
}

static
int doShow(const String &dbName) {
	ContentDbase *cdb = new ContentDbase;
	ifstream f(dbName.cstr());
	IBStream is;
	is.configure(&f, dbName);
	cdb->load(is);
	if (is.good()) {
		cdb->print(cout);
		return 0;
	}

	cerr << ThePrgName << ": error loading db from `" << dbName << "'" << endl;
	return -2;
}

static
int doAdd(const String &dbName, const Array<String*> &fnames) {
	ContentDbase *cdb = new ContentDbase;

	// we are using fstream for both reading and writing, and
	// in this case we need this additional code to make sure
	// that the file gets created if it does not exist yet
	{
	    // should be enough to create new file or keep existing
	    // one as it is
	    ofstream out(dbName.cstr(), ios::app);
	}

	fstream f(dbName.cstr(), ios::in|ios::out);

	{
		clog << ThePrgName << ": loading db from " << dbName << " ... ";
		IBStream is;
		is.configure(&f, dbName);
		cdb->load(is);
		clog << "\t " << setw(6) << cdb->count() << " objects" << endl;
	}

	if (!fnames.empty()) {
		for (int i = 0; i < fnames.count(); ++i) {
			const String &fname = *fnames[i];
#if HAVE_STAT
			// check whether path is a regular file
			struct stat fileStat;
			if (stat(fname.cstr(), &fileStat)) {
				cerr << ThePrgName << ": stat failed for '" <<
					fname << "': " << Error::Last() << endl;
				continue;
			}
			if (!S_ISREG(fileStat.st_mode)) {
				cerr << ThePrgName << ": skipping '" << fname <<
					"': not a regular file" << endl;
				continue;
			}
#endif // HAVE_STAT
			Must(readCont(cdb, fname));
		}
	} else {
		Must(readCont(cdb, "-"));
	}

	clog << ThePrgName << ": got " << fnames.count() << " files and " <<
		TheTotalLinkCount << " links" << endl;

	{
		clog << ThePrgName << ": storing db in " << dbName << " ... ";
		f.clear();
		f.seekg(0);
		OBStream os;
		os.configure(&f, dbName);
		cdb->store(os);
		if (os.good())
			clog << "\t " << setw(6) << cdb->count() << " objects" << endl;
		else
			clog << "error: " << Error::Last() << endl;
	}
	f.close();
	return 0;
}

// dump verbatim and linkonly objects from database, one object per file
static
int doDump(const String &dbName, const String &dumpPath) {
	ContentDbase cdb;
	ifstream f(dbName.cstr());

	{
		IBStream is;
		is.configure(&f, dbName);
		cdb.load(is);
		if (!is.good()) {
			cerr << ThePrgName << ": error loading db from '" << dbName << "'" << endl;
			return -2;
		}
	}

	bool error = false;
	int fileCounter = 0;
	int linkonlyObjCounter = 0;
	int verbatimObjCounter = 0;

	for (int i = 0; i < cdb.count(); ++i) {
		if (cdb.entry(i)->type() == cdbePage || // dumps for link-only objects
			cdb.entry(i)->type() == cdbeBlob) { // dumps for verbatim objects
			const String fileName = ExpandMacro(dumpPath, "${name}", AnyToString(fileCounter + 1));

			if (!access(fileName.cstr(), F_OK)) {
				error = true;
				cerr << ThePrgName << ": file '" << fileName << "' already exists" << endl;
				break;
			}
			if (Error::Last() != ENOENT) {
				error = true;
				cerr << ThePrgName << ": could not access '" << fileName << "'" << endl;
				break;
			}

			ofstream f(fileName.cstr());
			cdb.entry(i)->print(f);

			if (!f) {
				error = true;
				cerr << ThePrgName << ": error dumping object to '" << fileName << "'" << endl;
				break;
			}

			++fileCounter;
			if (cdb.entry(i)->type() == cdbePage)
				++linkonlyObjCounter;
			else
				++verbatimObjCounter;
		}
	}

	clog << ThePrgName << ": dumped " << (fileCounter) << " objects ("
		<< linkonlyObjCounter << " linkonly, " << verbatimObjCounter
		<< " verbatim), skipped " << (cdb.count() - fileCounter)
		<< " objects" << endl;
	return error ? -1 : 0;
}

int main(int argc, char *argv[]) {
	ThePrgName = argv[0];
	CmdLine cmd;

	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	int result = -1;
	if (TheOpts.theAddDb)
		result = doAdd(TheOpts.theAddDb, TheOpts.theFiles);
	else
	if (TheOpts.theDumpDb)
		result = doDump(TheOpts.theDumpDb, TheOpts.theAsPath);
	else
	if (TheOpts.theShowDb)
		result = doShow(TheOpts.theShowDb);

	return result;
}
