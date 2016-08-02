
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include <fstream>
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Ring.h"
#include "base/CmdLine.h"
#include "base/StatIntvlRec.h"
#include "base/polyOpts.h"
#include "logextractors/LogIter.h"
#include "logextractors/LogCatFilter.h"

#include "base/opts.h"
#include "logextractors/matchAndPrint.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "xstd/gadgets.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,     "help",         "list of options"),
			theVersOpt(this,     "version",      "package version info"),
			theOutFileName(this, "out <file>",   "redirect console output", "-"),
			theSideName(this,    "side <clt|srv|all>", "name of `side' to extract"),
			theObjects(this,     "objects <list>","names of objects to extract"),
			theWinLen(this,      "win_len <time>","averaging window length", Time::Sec(60)),
			theTimeUnit(this,    "time_unit <time>","use relative time and given time unit for `time' object"),
			syncTimes(this,      "sync_times <bool>","adjust local log time as if all logs started at once", false),
			smoothSlide(this,    "smooth_slide <bool>","slide averaging window one log entry at a time", false)
			{}

		virtual ostream &printAnonym(ostream &os) const;
		virtual bool parseAnonym(const Array<const char *> &opts);
		virtual bool canParseAnonym() const { return true; }

		virtual bool validate() const;

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theOutFileName;
		StrOpt theSideName;
		StrArrOpt theObjects;
		TimeOpt theWinLen;
		TimeOpt theTimeUnit;
		BoolOpt syncTimes;
		BoolOpt smoothSlide;
		PtrArray<String*> theFiles;
};

struct TraceIntvlRec {	
	StatIntvlRec rec;
	Time time;

	TraceIntvlRec() {}
	TraceIntvlRec(Time tm, const StatIntvlRec &r): rec(r), time(tm) {}
};

class TraceWin: protected Ring<TraceIntvlRec*> {
	public:
		TraceWin(const String &fname, Time aLength);
		~TraceWin();

		void startTime(Time time);
		int avg(StatIntvlRec &rec, Time &min, Time &max) const;
		void clean(Time coveredTime);
		void offset(Time anOffset) { theTimeOff = anOffset; }

		Time timeBeg() const { return empty() ? Time() : beg().time; }
		Time timeEnd() const { return empty() ? Time() : end().time; }

		const String &phaseName() const { return thePhaseName; }

		void step(Time intvlBeg);

	protected:
		const TraceIntvlRec &beg() const { return *top(); }
		const TraceIntvlRec &end() const { return *top(count() - 1); }
		const TraceIntvlRec &tir(const int off) const { return *top(off); }
		TraceIntvlRec &tir(const int off) { return *top(off); }

		void doStep();

		bool load(const LogEntryPx &px);
		void add(Time tm, const StatIntvlRec &rec);

	protected:
		ILog theLog;
		LogIter theLogIter;

		Time theLength;
		Time theTimeOff;     // to correct local time if needed
		Time theIntvlBeg;
		Time theIntvlEnd;

		String thePhaseName;  // current phase name
};

class MyScanner {
	protected:
		typedef const Array<String*> &Strs;

	public:
		MyScanner(Strs objectNames, Strs fileNames);
		~MyScanner();

		void run();

	protected:
		int step();
		Time getIntvlBeg() const;

		ostream &printTime(ostream &os, Time tm) const;
		void report(const String &name, const StatIntvlRec &rec, int count, Time min, Time max) const;

	protected:
		Strs theObjectNames;
		Time theTimeStart;    // smallest time among all logs
		Time theIntvlBeg;
		Time theIntvlEnd;

		Array<TraceWin*> theWins;
};

static MyOpts TheOpts;
static LogCatFilter *TheFilter = 0;

/* MyOpt */

ostream &MyOpts::printAnonym(ostream &os) const {
	return os << "<log_file_name> ...";
}

bool MyOpts::parseAnonym(const Array<const char *> &opts) {
	for (int i = 0 ; i < opts.count(); ++i)
		theFiles.append(new String(opts[i]));

	// add default fname if none are specified
	if (!theFiles.count())
		theFiles.append(new String("-"));

	return theFiles.count() > 0;
}

bool MyOpts::validate() const {
	if (!theObjects)
		cerr << "tracing _all_ objects may produce huge output and is not supported" << endl;
	else
	if (theSideName && theSideName != "clt" && theSideName != "srv" && theSideName != "all")
		cerr << "side name must be `clt' or `srv' or `all'; got: " << theSideName << endl;
	else
		return OptGrp::validate();
	return false;
}

/* TraceWin */

TraceWin::TraceWin(const String &fname, Time aLength):
	theLength(aLength), theTimeOff(0,0) {
	if (fname == "-")
		theLog.stream("stdin", &cin);
	else
		theLog.stream(fname, (istream*)new ifstream(fname.cstr(), ios::binary|ios::in));
	theLogIter.start(&theLog);
	doStep();
}

TraceWin::~TraceWin() {
	while (!empty()) delete dequeue();
}

void TraceWin::startTime(Time time) {
	Assert(count() == 1);
	theTimeOff = time - timeBeg();
	tir(0).time += theTimeOff;
}

void TraceWin::step(Time intvlBeg) {
	theIntvlBeg = intvlBeg;
	theIntvlEnd = intvlBeg + theLength;
	while (theLogIter && timeEnd() < theIntvlEnd)
		doStep();
}

int TraceWin::avg(StatIntvlRec &rec, Time &min, Time &max) const {
	StatIntvlRec accum;
	int accumCount = 0;
	for (; accumCount < count(); ++accumCount) {
		const TraceIntvlRec &r = tir(accumCount);
		if (theIntvlBeg <= r.time && r.time < theIntvlEnd)
			accum.concat(r.rec);
		else
			break;
	}

	if (accumCount) {
		rec.merge(accum);
		min = min < 0 ? timeBeg() : Min(min, timeBeg());
		max = Max(max, tir(accumCount-1).time);
	}
	return accumCount;
}

// get rid of old entries
void TraceWin::clean(Time coveredTime) {
	while (!empty() && timeBeg() <= coveredTime)
		delete dequeue();

	if (empty())
		doStep();
}

void TraceWin::doStep() {
	bool found = false;
	while (!found && theLogIter) {
		found = load(*theLogIter);
		++theLogIter;
	}
}

bool TraceWin::load(const LogEntryPx &px) {
	if (!TheFilter->passed(px))
		return false;

	switch (px.theTag) {
		case lgContTypeKinds: {
			// should be called only once per log
			ContType::Load(theLog);
			break;
		}
		case lgStatCycleRec: {
			StatIntvlRec r;
			r.load(theLog);
			if (!r.sane()) {
				clog << theLog.fileName() << ':' << theLog.pos()
					<< ": warning: skipping corrupted entry"
					<< " (log time: " << theLog.progress().time() << ')' << endl;
				return false;
			}
			add(theLog.progress().time() + theTimeOff, r);
			return true;
		}
		case lgStatPhaseBeg: {
			theLog >> thePhaseName;
			return false; // no stats loaded
		}
	}
	return false;
}

void TraceWin::add(Time tm, const StatIntvlRec &rec) {
	if (full())
		resize(1 + 2*capacity());
	Assert(!full());

	enqueue(new TraceIntvlRec(tm, rec));
}


/* MyScanner */

MyScanner::MyScanner(Strs objectNames, Strs fileNames):
	theObjectNames(objectNames) {

	for (int i = 0; i < fileNames.count(); ++i) {
		const String fname = *fileNames[i];
		TraceWin *win = new TraceWin(fname, TheOpts.theWinLen);
		if (win->timeBeg() < 0) {
			cerr << fname << ":warning: failed to read log file, skipping" << endl;
			continue;
		}
		theWins.append(win);
	}

	if (!theWins.count()) {
		cerr << "failed to read all " << fileNames.count() << " logs, exiting" << endl;
		exit(-1);
	}

	if (TheOpts.syncTimes) {
		theIntvlBeg = getIntvlBeg();
		for (int i = 0; i < theWins.count(); ++i)
			theWins[i]->startTime(theIntvlBeg);
	}
}

MyScanner::~MyScanner() {
	while (theWins.count()) delete theWins.pop();
}


void MyScanner::run() {
	do {
		step();
	} while (theIntvlBeg > 0);
}

int MyScanner::step() {
	theIntvlBeg = getIntvlBeg();
	if (theTimeStart < 0)
		theTimeStart = theIntvlBeg;

	StatIntvlRec rec;
	Time min, max;
	String phaseName = 0;

	int count = 0;
	{for (int i = 0; i < theWins.count(); ++i) {
		TraceWin &win = *theWins[i];
		win.step(theIntvlBeg);
		count += win.avg(rec, min, max);
		if (!phaseName)
			phaseName = win.phaseName();
	}}

	report(phaseName, rec, count, min, max);

	{for (int i = 0; i < theWins.count(); ++i) {
		TraceWin &win = *theWins[i];
		win.clean(TheOpts.smoothSlide ? min : max);
	}}
	return count;
}

ostream &MyScanner::printTime(ostream &os, Time tm) const {
	if (TheOpts.theTimeUnit > 0) {
		tm -= theTimeStart;
		os << (tm/TheOpts.theTimeUnit);
	} else {
		os << tm.secd();
	}
	return os;
}

void MyScanner::report(const String &name, const StatIntvlRec &rec, int count, Time min, Time max) const {
	if (!count)
		return;

	ostringstream buf;
	configureStream(buf, 2);

	const Time middle = min/2 + max/2; // avoid year-2038 tv_sec overflows
	printTime(buf << "time:\t ", middle) << endl;
	rec.print(buf, "");
	buf
		<< "interval.start:\t   " << min << endl
		<< "interval.end:\t     " << max << endl
		<< "interval.entries:\t " << count << endl;

	buf << "name:\t ";
	if (name)
		buf << name;
	else 
		buf << '?';
	buf << endl;

	buf << ends;

	static bool printHeader = true;
	if (printHeader) {
		if (MatchAndPrintHeader(cout, buf.str().c_str(), theObjectNames) == 0) {
			cerr << "no logged objects matched any of the requested object names" << endl;
			exit(-2);
		}
		printHeader = false;
	}
	MatchAndPrintBodies(cout, buf.str().c_str(), theObjectNames);
	streamFreeze(buf, false);
}

Time MyScanner::getIntvlBeg() const {
	Time beg;
	for (int i = 0; i < theWins.count(); ++i) {
		const Time b = theWins[i]->timeBeg();
		if (b < 0)
			continue;
		if (beg < 0 || beg > b)
			beg = b;
	}
	return beg;
}

/* local routines */

static
void configureLogs(int prec) {
	if (TheOpts.theOutFileName && TheOpts.theOutFileName != "-")
		redirectOutput(TheOpts.theOutFileName.cstr());

	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);
}

static
void configure() {
	configureLogs(2);

	int lc = lgcAll;
	if (TheOpts.theSideName == "clt")
		lc = lgcCltSide;
	else
	if (TheOpts.theSideName == "srv")
		lc = lgcSrvSide;
	TheFilter = new LogCatFilter;
	TheFilter->logCat(lc);
}

int main(int argc, char *argv[]) {

	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	configure();

	MyScanner scanner(TheOpts.theObjects.val(), TheOpts.theFiles);
	scanner.run();

	return 0;
}
