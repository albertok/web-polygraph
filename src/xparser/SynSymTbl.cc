
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xparser/xparser.h"

#include "xstd/String.h"
#include "xparser/SynSymTbl.h"

bool SynSymTbl::PrnUndef = true;


/* SynSymTblItem */

SynSymTblItem::SynSymTblItem(): theSym(0), theUseCnt(0) {
}

SynSymTblItem::SynSymTblItem(const String &aType, const String &aName):
	theSym(0), theName(aName), theType(aType), theUseCnt(0) {
}

SynSymTblItem::~SynSymTblItem() {
	delete theSym;
}

void SynSymTblItem::sym(SynSym *aSym) {
	theSym = aSym;
	if (theSym && !theSym->loc())
		theSym->loc(loc());
}


/* SynSymTbl */

SynSymTbl::SynSymTbl() {
}

SynSymTbl::~SynSymTbl() {
	for (int i = 0; i < count(); ++i)
		delete valAt(i);
}

void SynSymTbl::report(ostream &os, const String &pfx) const {

	for (int i = 0; i < count(); ++i) {
		if (valAt(i)) {
			if (!PrnUndef && !valAt(i)->sym())
				continue;
			os << pfx << valAt(i)->name() << " = ";
			if (valAt(i)->sym())
				valAt(i)->sym()->print(os, pfx);
			else
				os << "undef()";
			os << ';' << endl;
		}
	}
}

void SynSymTbl::briefReport(ostream &os, const String &pfx) const {
	report(os, pfx, false);
}

void SynSymTbl::report(ostream &os, const String &pfx, bool prnUndef) const {
	const bool savedPrnUndef = PrnUndef;
	PrnUndef = prnUndef;
	report(os, pfx);
	PrnUndef = savedPrnUndef;
}
