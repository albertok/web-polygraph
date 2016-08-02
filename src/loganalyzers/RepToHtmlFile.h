
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_REPTOHTMLFILE_H
#define POLYGRAPH__LOGANALYZERS_REPTOHTMLFILE_H

#include "xstd/Map.h"
#include "xstd/String.h"
#include "xml/XmlRenderer.h"
#include "loganalyzers/SectionState.h"

class BlobDb;

class RepToHtmlFile: public XmlRenderer {
	public:
		static void Location(BlobDb &db, const ReportBlob &blob, const String &fname);
		static void CollectLocations(BlobDb &db, const XmlNode &node, const String &fname);
		static String Location(const String &key);

	public:
		RepToHtmlFile(BlobDb &db, ostream *aStream, const String &aLocation);
		virtual ~RepToHtmlFile();

		virtual void render(const XmlDoc &doc);
		virtual void renderReportBlob(const ReportBlob &blob);
		virtual void renderText(const char *buf, Size sz);
		virtual void renderTag(const XmlTag &tag);

	protected:
		void renderDocument(const XmlTag &tag);
		void renderChapter(const XmlTag &tag);
		void renderSection(const XmlTag &tag);
		void renderBlobInclude(const XmlTag &tag);
		void renderBlobPtr(const XmlTag &tag);
		void renderBlob(const XmlTag &tag);
		void renderMeasurement(const XmlTag &tag);
		void renderMeasurementVal(const XmlTag &tag, const String &val, bool renderUnit, const String &unit);
		void renderList(const XmlTag &tag);
		void renderTableCell(const XmlTag &tag);
		void renderImage(const XmlTag &tag);
		void renderSampleStart(const XmlNode &n, const String &element, const String &typeId);

		String relativeUrl(const String &from, const String &to) const;
		String location(const String &key) const;

		static String JavascriptInclude(const String &file);
		static String StylesheetInclude(const String &file, const String &media = "");

	protected:
		static PtrMap<String*> TheLocations; // global file names

		BlobDb &theDb;
		ostream *theStream;
		String theLocation;
		Array<const XmlNode*> theParents; // used to link out-of-tree blobs
		int theQuoteLevel;

		SectionState theSectionState;
};

#endif
