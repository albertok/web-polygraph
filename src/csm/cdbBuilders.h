
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CDBBUILDERS_H
#define POLYGRAPH__CSM_CDBBUILDERS_H

class ContentDbase;
class CdbEntry;
class CdbePage;
class PrefixIdentifier;

class CdbBuilder {
	public:
		CdbBuilder();
		virtual ~CdbBuilder();

		void db(ContentDbase *aDb);
		void configure(const String &aFname, const char *aBufB, const char *aBufE);
		virtual bool parse() = 0;

	public:
		static int TheLinkCount;

	protected:
		ContentDbase *theDb;
		String theFname;
		const char *theBufB;
		const char *theBufE;
};

class MarkupParser: public CdbBuilder {
	public:
		virtual bool parse();
		virtual void addEntry(CdbEntry *e);

	protected:
		void parseTag(const String &image);
		void parseBlob(const String &blobImage);

		int RegReplacement(const String &key, const String &ctype);
		const String *AttrValReplacement(const String &keyPfx, const String &keySfx);

	private:
		static PrefixIdentifier *TheReplIdentifier;
		static Array<String*> TheReplacements;
};

class LinkOnlyParser: public MarkupParser {
	public:
		LinkOnlyParser();

		virtual bool parse();
		virtual void addEntry(CdbEntry *e);

	protected:
		void flush();

	private:
		CdbePage *thePage;
		String theImage;
};

class VerbatimParser: public CdbBuilder {
	public:
		virtual bool parse();
};

#endif
