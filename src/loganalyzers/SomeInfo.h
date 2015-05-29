
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_SOMEINFO_H
#define POLYGRAPH__LOGANALYZERS_SOMEINFO_H

#include "loganalyzers/InfoScope.h"

class String;
class BlobDb;
class ReportBlob;
class XmlAttr;

// common base for all report *Info classes
class SomeInfo {
	public:
		typedef InfoScope Scope;

	public:
		virtual ~SomeInfo() {}

	protected:
		const ReportBlob &addLink(BlobDb &db, const String &newKey, const String &oldKey);

		const ReportBlob &addMeasBlob(BlobDb &db, const String &name, double val, const String &unit, const String &title);
		const ReportBlob &addMeasBlob(BlobDb &db, const String &name, Time val, const String &title);
		const ReportBlob &addMeasBlob(BlobDb &db, const String &name, const String &val, const String &unit, const String &title);

		const ReportBlob &addNaMeasBlob(BlobDb &db, const String &name, const String &title);
};

#endif
