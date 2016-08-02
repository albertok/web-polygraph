
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/sstream.h"

#include "base/AnyToString.h"
#include "xml/XmlAttr.h"
#include "xml/XmlText.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"

static const String strKey = "key";

String BlobDb::Key(const String &name, const Scope &scope) {
	return name + KeySuffix(scope);
}

String BlobDb::KeySuffix(const Scope &scope) {
	return scope ? String(".scope=") + scope.image() : String("");
}

BlobDb::BlobDb(): theBlobIdx(15991), theNotesCount(0) {
}

const ReportBlob *BlobDb::has(const String &key) {
	return find(key);
}

bool CheckParent(const XmlNode &n) {
	if (n.kids()) {
		for (int i = 0; i < n.kids()->count(); ++i) {
//clog << here << n.kids()->item(i)->name() << endl;
			Assert(n.kids()->item(i)->parent());
			if (!CheckParent(*n.kids()->item(i))) {
				Assert(false);
				return false;
			}
		}
	}
	return true;
}

const ReportBlob *BlobDb::add(const ReportBlob &b) {
	//clog << here << "created: " << b.key() << endl;

	//CheckParent(b);

	if (const ReportBlob *copy = find(b.key())) {
		//cerr << here << "duplicate: " << b.key() << endl;
		ostringstream buf1, buf2;
		b.print(buf1, "") << ends;
		copy->print(buf2, "") << ends;
		if (buf1.str() != buf2.str())
			cerr << "internal_error: inconsistent computation of " << b.key() << endl;
		streamFreeze(buf1, false);
		streamFreeze(buf2, false);
		return copy;
	} else {
		theBlobs << b;
		theBlobIdx.add((const ReportBlob*)theBlobs.last());
		return (const ReportBlob*)theBlobs.last();
	}
}

const ReportBlob &BlobDb::get(const String &key) {
	//cerr << here << "looking for blob: " << key << endl;
	const char *descr = 0;

	if (const ReportBlob *blob = find(key))
		return *blob;

	XmlSearchRes errs;
	theErrors.selByAttrVal(strKey, key, errs);
	if (errs.count())
		return *(const ReportBlob*)errs.last();

	cerr << here << "error: cannot find blob: " << key << endl;

	// create an error blob so that we can return something
	ReportBlob blob(key, "internal reporter error");
	XmlTag err("internal_error");
	if (descr)
		err << XmlAttr("description", "blob not found");
	err << XmlAttr("context", key);
	blob << err;
	theErrors << blob;
	return *(const ReportBlob*)theErrors.last();
}

// XXX: check that include is used at most once per key
const XmlNode &BlobDb::include(const String &key) {
	XmlTag t("include");
	t << XmlAttr("src", key);
	t << XmlAttr("auth", "1"); // authoritative
	theIncludes << t;
	return *theIncludes.last();
}

const XmlNode &BlobDb::quote(const String &key) {
	XmlTag t("include");
	t << XmlAttr("src", key);
	theIncludes << t;
	return *theIncludes.last();
}

XmlNode &BlobDb::ptr(const String &key, const XmlNodes &context) {
	XmlTag t("blob_ptr");
	t << XmlAttr(strKey, key);
	t << context;
	thePtrs << t;
	return *thePtrs.last();
}

XmlNode &BlobDb::nullPtr(const XmlNodes &context) {
	XmlTag t("blob_ptr");
	t << XmlAttr::Int("maybe_null", true);
	t << context;
	thePtrs << t;
	return *thePtrs.last();
}

XmlTag BlobDb::reportNote(const String &id, const XmlNode &note) {
	static const String key("notes.");

	XmlNode *node = theBlobs.findByAttrVal("report_note", id);
	String number;
	if (node) {
		number = node->attrs()->value("report_note_number");
		if (!node->attrs()->has("many_occurrences")) {
			node->addAttr(new XmlAttr("many_occurrences", ""));
			node->addChild(new XmlText(" (many occurrences)"));
		}
	} else {
		static const String title("report note ");

		number = AnyToString(++theNotesCount);
		ReportBlob blob(key + number, title + number);
		blob << XmlAttr("report_note", id);
		blob << XmlAttr("report_note_number", number);
		blob << note;
		add(blob);
	}

	XmlTag t("sup");
	t << ptr(key + number, XmlText("[" + number + "]"));
	return t;
}

const ReportBlob *BlobDb::find(const String &key) const {
	return theBlobIdx.find(key);
}

ostream &BlobDb::print(ostream &os, const String &pfx) const {
	return theBlobs.print(os, pfx);
}
