
/* Web Polygraph       http://www.web-polygraph.org/
 * (C) 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglNumSym.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"

#include "pgl/MimeHeaderSym.h"


const String MimeHeaderSym::TheType = "MimeHeader";

static const String strName = "name";
static const String strValue = "value";

MimeHeaderSym::MimeHeaderSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(StringSym::TheType, strName, 0);
	theRec->bAdd(StringSym::TheType, strValue, 0);
}

MimeHeaderSym::MimeHeaderSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

MimeHeaderSym *MimeHeaderSym::Parse(const String &s) {
	MimeHeaderSym *sym = 0;
	const int pos = s.find(':');
	if (pos != String::npos &&
		s[0] != ':' &&
		pos + 1 < s.len() &&
		s[pos + 1] == ' ') {
		sym = new MimeHeaderSym;
		Assert(sym->theRec);
		SynSymTblItem *name = 0;
		Assert(sym->theRec->find(strName, name));
		const String nameVal = s(0, pos);
		name->sym(new StringSym(nameVal));
		if (pos + 2 < s.len()) {
			SynSymTblItem *value = 0;
			Assert(sym->theRec->find(strValue, value));
			const String valueVal = s(pos + 2, String::npos);
			value->sym(new StringSym(valueVal));
		}
	}
	return sym;
}

bool MimeHeaderSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *MimeHeaderSym::dupe(const String &type) const {
	if (isA(type))
		return new MimeHeaderSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

const String *MimeHeaderSym::name() const {
	SynSymTblItem *ni = 0;
	Assert(theRec->find(strName, ni));
	return ni->sym() ?
		&((const StringSym&)ni->sym()->cast(StringSym::TheType)).val() : 0;
}

const String *MimeHeaderSym::value() const {
	SynSymTblItem *ni = 0;
	Assert(theRec->find(strValue, ni));
	return ni->sym() ?
		&((const StringSym&)ni->sym()->cast(StringSym::TheType)).val() : 0;
}
