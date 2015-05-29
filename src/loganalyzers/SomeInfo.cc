
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xml/XmlTag.h"
#include "xml/XmlText.h"
#include "xml/XmlAttr.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/Sample.h"
#include "loganalyzers/SomeInfo.h"


class MeasBlob: public ReportBlob {
	public:
		MeasBlob(const String &key, const String &title);

		void value(const XmlAttr &val);
		void unit(const String &unit);
		void typeId(const String &typeId);

	public:
		XmlNode *measurement;
};

MeasBlob::MeasBlob(const String &key, const String &title): 
	ReportBlob(key, title), measurement(0) {
	static const XmlAttr dtype("dtype", "span");
	*this << dtype;
	static const String tagName = "measurement";
	measurement = append(XmlTag(tagName));
}

void MeasBlob::value(const XmlAttr &val) {
	*measurement->attrs() << val;
}

void MeasBlob::unit(const String &unit) {
	static String attrName = "unit";
	*measurement->attrs() << XmlAttr(attrName, unit);
}

void MeasBlob::typeId(const String &typeId) {
	static String attrName = "typeId";
	*measurement->attrs() << XmlAttr(attrName, typeId);
}


/* SomeInfo */

const ReportBlob &SomeInfo::addLink(BlobDb &db, const String &newKey, const String &oldKey) {
	ReportBlob blob(newKey, ReportBlob::NilTitle);
	blob << db.include(oldKey);
	return *db.add(blob);
}

const ReportBlob &SomeInfo::addMeasBlob(BlobDb &db, const String &name, double val, const String &unit, const String &title) {
	MeasBlob blob(name, title);
	blob.value(XmlAttr::Double("value", val));
	blob.unit(unit);
	blob.typeId(NumberSample::TheId);
	const ReportBlob &res = *db.add(blob);
	return res;
}

const ReportBlob &SomeInfo::addMeasBlob(BlobDb &db, const String &name, Time val, const String &title) {
	MeasBlob blob(name, title);
	blob.value(XmlAttr::Double("value", val.secd()));
	blob.unit("sec");
	blob.typeId(NumberSample::TheId);

	XmlTag image("image");
	XmlText text;
	text.buf() << val;
	image << text;
	blob.measurement->addChild(image.clone());
	return *db.add(blob);
}

const ReportBlob &SomeInfo::addMeasBlob(BlobDb &db, const String &name, const String &val, const String &unit, const String &title) {
	MeasBlob blob(name, title);
	blob.value(XmlAttr("value", val));
	blob.unit(unit);
	blob.typeId(TextSample::TheId);
	return *db.add(blob);
}

const ReportBlob &SomeInfo::addNaMeasBlob(BlobDb &db, const String &name, const String &title) {
	static const XmlText textNote("no measurement was collected or stored at run-time");
	MeasBlob measBlob(name, title);
	measBlob << XmlText("n/a") << db.reportNote("n/a", textNote);
	return *db.add(measBlob);
}
