
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include <fstream>

#include "xstd/String.h"
#include "xstd/Rnd.h"
#include "xstd/TblDistr.h"
#include "xstd/Error.h"
#include "xstd/SrcLoc.h"
#include "xstd/gadgets.h"
#include "base/loadTblDistr.h"

static SrcLoc TheLoc;

static
void ltrim(char *&p) {
	while (isspace(*p)) ++p;
}

static
void rtrim(char *p) {
	int len = strlen(p);
	while (len && isspace(p[len-1])) --len;
	p[len] = 0;
}

inline
void trim(char *&p) {
	ltrim(p);
	rtrim(p);
}

static
int getLine(istream &is, Array<char*> &line) {

	line.reset();

	static char buf[8*1024];
	while (!line.count() && is && is.getline(buf, sizeof(buf))) {
		char *p = buf;

		// delete comments
		if (char *c = strchr(p, '#'))
			*c = 0;

		// split into "words"
		ltrim(p);
		while (strlen(p)) {
			//cerr << here << "{ p = '" << p << "'" << endl;
			char *eow = p;
			while (*eow && !isspace(*eow)) ++eow;

			if (p < eow) {
				line.append(p);
				p = *eow ? eow + 1 : eow;
				*eow = '\0';
			}

			ltrim(p);
		}
		++TheLoc;
	}

	/*cerr << here << line.count() << ": ";
	for (int i = 0; i < line.count(); ++i) cerr << " '" << line[i] << "'";
	cerr << endl;*/

	return line.count();
}

static
RndDistr *loadFailure(TblDistr *&tbl) {
	delete tbl;
	return tbl = 0;
}

static
bool matchWord(const String &expected, const String &got) {
	if (expected != got) {
		cerr << TheLoc << "expected `" << expected 
			<< "', got `" << got << "'" << endl;
		return false;
	}
	return true;
}

static
bool getNumber(double &num, const String &str) {
	if (!isNum(str.cstr(), num)) {
		cerr << TheLoc << "expected a number, got `" << str << "'" << endl;
		return false;
	}
	return true;
}

RndDistr *LoadTblDistr(const String &fname, String &distrType) {
	Assert(fname);

	clog << "loading distribution pdf from '" << fname << "'..." << endl;
	TheLoc = SrcLoc(fname, 0);

	TblDistr *tbl = 0;

	ifstream is(fname.cstr());
	Array<char*> line;

	if (!is) {
		cerr << "error: cannot open '" << fname << "': " << Error::Last() << endl;
		return loadFailure(tbl);
	}

	// interpret header
	if (getLine(is, line) == 4) {
		const String loadedDistrType = line.shift();
		if (distrType.len() && !matchWord(distrType, loadedDistrType))
		    return loadFailure(tbl);

		distrType = loadedDistrType; // may already be the same

		tbl = new TblDistr(line.shift()); // name

		if (!matchWord("=", line.shift()))
			return loadFailure(tbl);
		if (!matchWord("{", line.shift()))
			return loadFailure(tbl);

	} else {
		cerr << TheLoc << "malformed or missing header line for a table distribution" << endl;
		return loadFailure(tbl);
	}

	Assert(tbl);
	
	TblDistrBin *wild_card = 0;
	double setContrib = -1;
	double accContrib = 0;
	double next_sup = 0;

	// load bins until end-of-bins symbol is found
	const String eob = "}";
	while (getLine(is, line)) {

		if (line.count() == 1 && eob == line[0])
			break;

		if (line.count() != 2) {
			cerr << TheLoc << "syntax error, expected `<min:max> <contribution>'" << endl;
			return loadFailure(tbl);
		}

		TblDistrBin *bin = 0;
		String h = line.shift();
		const char *column = h.chr(':');
		if (column) {
			double min, max;

			if (h[0] == ':') {
				min = next_sup;
				if (!getNumber(max, column+1))
					return loadFailure(tbl);
			} else {
				if (!getNumber(min, h))
					return loadFailure(tbl);
				if (!getNumber(max, column+1))
					return loadFailure(tbl);
			}

			if (min >= max) {
					cerr << TheLoc << "min must be smaller than max" << endl;
					return loadFailure(tbl);
			}

			bin = new TblDistrIntvl(min, max);
			next_sup = max;

		} else {
			double point;
			if (!getNumber(point, h))
				return loadFailure(tbl);
			bin = new TblDistrPoint(point);
			next_sup = point + 1;
		}
		Assert(bin);

		if (tbl->overlapping(bin)) {
			cerr << TheLoc << "bin overlaps with earlier bin(s)" << endl;
			tbl->printBins(cerr);
			delete bin;
			return loadFailure(tbl);
		}

		h = line.shift();
		if (h[0] == '*') {
			if (wild_card) {
				cerr << TheLoc << "only one wild card per pdf is allowed" << endl;
				return loadFailure(tbl);
			}
			wild_card = bin;
			const char *slash = h.chr('/');
			if (slash) {
				if (!getNumber(setContrib, slash+1))
					return loadFailure(tbl);
			} else {
				cerr << TheLoc << "wild card entry requires"
					<< " total contribution field (e.g., '*/100')" << endl;
				return loadFailure(tbl);
			}
		} else {
			double contrib;
			if (!getNumber(contrib, h))
				return loadFailure(tbl);

			if (contrib > 0) { // ignore non-contributing bins
				bin->absContrib(contrib);
				tbl->addBin(bin);
				accContrib += contrib;
			} else
				delete bin;
		}
	}

	is.close();

	if (wild_card) {
		if (accContrib > setContrib) {
			cerr << TheLoc << "specified total contribution of "
				<< setContrib 
				<< " is less than actual contribution of "
				<< accContrib << endl;
			delete wild_card;
			return loadFailure(tbl);
		}
		const double contrib = setContrib - accContrib;
		if (contrib > 0) {
			wild_card->absContrib(contrib);
			tbl->addBin(wild_card);
		} else	
			delete wild_card;
	}

	if (!tbl->binCount()) {
		cerr << TheLoc << ": error: empty pdf declaration" << endl;
		return loadFailure(tbl);
	}

	tbl->make();

	clog << TheLoc 
		<< "loaded `" << *tbl << "'"
		<< " idx entries: " << tbl->idxCount()
		<< " idx depth: " << tbl->idxDepth()
		<< endl;

	return tbl;
}
