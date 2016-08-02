
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_BLOBDB_H
#define POLYGRAPH__LOGANALYZERS_BLOBDB_H

#include "xml/XmlNodes.h"
#include "xml/XmlTag.h"
#include "loganalyzers/InfoScope.h"
#include "loganalyzers/BlobIdx.h"

class ReportBlob;

// a collection of ReportBlobs
class BlobDb {
	public:
		typedef InfoScope Scope;

	public:
		static String Key(const String &name, const Scope &scope);
		static String KeySuffix(const Scope &scope);

	public:
		BlobDb();

		const ReportBlob *has(const String &key);
		const ReportBlob *add(const ReportBlob &b);
		BlobDb &operator <<(const ReportBlob &b) { add(b); return *this; }

		const XmlNodes &blobs() const { return theBlobs; }

		// never fail
		void link(const String &oldString, const String &newString);
		const ReportBlob &get(const String &key);
		const XmlNode &include(const String &key); // at most once
		const XmlNode &quote(const String &key);   // many times
		XmlNode &ptr(const String &key, const XmlNodes &context); // link
		XmlNode &nullPtr(const XmlNodes &context); // grayed-out link
		XmlTag reportNote(const String &id, const XmlNode &note);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		const ReportBlob *find(const String &key) const;

	protected:
		XmlNodes theBlobs;
		BlobIdx theBlobIdx;
		XmlNodes theIncludes;
		XmlNodes thePtrs;
		XmlNodes theErrors;
		int theNotesCount;
};

inline
String operator +(const String &name, const InfoScope &scope) {
        return BlobDb::Key(name, scope);
}

inline
String operator +(const char *name, const InfoScope &scope) {
        return String(name) + scope;
}


#endif
