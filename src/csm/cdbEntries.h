
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CDBENTRIES_H
#define POLYGRAPH__CSM_CDBENTRIES_H

#include "xstd/Area.h"

class StrIdentifier;
class RndGen;
class ContentDbase;

enum { cdbeNone = 0, cdbeText, cdbeBlob, cdbeComment, cdbeLink, 
	cdbePage, cdbeEnd };

class CdbEntryPrnOpt {
	public:
		class Embed {
			public:
				Embed(): model(0), rng(0), count(0) {}

			public:
				EmbedContMdl *model;
				RndGen *rng;
				ObjId container;
				int count;
		};

	public:
		CdbEntryPrnOpt();

	public:
		WrBuf *buf;
		InjectIter *injector;
		Size sizeMax;
		Size entryOff;
		int entryData; // maintained by entries

		Embed embed;
};

class CdbEntry {
	public:
		virtual ~CdbEntry() {}

		virtual int type() const = 0;
		virtual Size size(CdbEntryPrnOpt &opt) const = 0;
		virtual Size meanSize() const = 0;

		virtual IBStream &load(IBStream &il) = 0;
		virtual OBStream &store(OBStream &ol) const = 0;

		virtual ostream &print(ostream &os) const = 0;
		virtual bool pour(CdbEntryPrnOpt &opt, bool &needMore) const = 0;

	protected:
		static bool Pour(const Area &image, bool divisible, CdbEntryPrnOpt &opt, bool &needMore);
};

// opaque, unbreakable piece of data
class CdbeBlob: public CdbEntry {
	public:
		virtual int type() const { return cdbeBlob; }
		virtual Size size(CdbEntryPrnOpt &) const { return theImage.len(); }
		virtual Size meanSize() const { return theImage.len(); }

		void image(const String &image);

		virtual OBStream &store(OBStream &ol) const;
		virtual IBStream &load(IBStream &il);

		virtual ostream &print(ostream &os) const;
		virtual bool pour(CdbEntryPrnOpt &opt, bool &needMore) const;

	public:
		String theImage;
};


// breakable piece of text, can take injections
class CdbeText: public CdbeBlob {
	public:
		virtual int type() const { return cdbeText; }

		virtual bool pour(CdbEntryPrnOpt &opt, bool &needMore) const;
};

// unbreakable text inside <!-- -->
// need to add inside-breakability and inside-injection functionalities
class CdbeComment: public CdbeBlob {
	public:
		virtual int type() const { return cdbeComment; }
};

// a link to another document (e.g., the value of a HTML src attribute)
// the image is created runtime based on server world ID and such
class CdbeLink: public CdbEntry {
	public:
		virtual int type() const { return cdbeLink; }
		virtual Size size(CdbEntryPrnOpt &opt) const;
		virtual Size meanSize() const;

		virtual OBStream &store(OBStream &ol) const;
		virtual IBStream &load(IBStream &il);

		virtual ostream &print(ostream &os) const;
		virtual bool pour(CdbEntryPrnOpt &opt, bool &needMore) const;

	public:
		String contentCategory;
		String origImage;

	protected:
		Area generateImage(CdbEntryPrnOpt::Embed &e) const;
};

// a page containing other cdb entries
class CdbePage: public CdbEntry {
	public:
		CdbePage();
		virtual ~CdbePage();

		virtual int type() const { return cdbePage; }
		virtual Size size(CdbEntryPrnOpt &opt) const;
		virtual Size meanSize() const;

		void add(CdbEntry *e);

		virtual OBStream &store(OBStream &ol) const;
		virtual IBStream &load(IBStream &il);

		virtual ostream &print(ostream &os) const;
		virtual bool pour(CdbEntryPrnOpt &opt, bool &needMore) const;

	private:
		ContentDbase *theDb;
};

#endif
