
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_REPORTBLOB_H
#define POLYGRAPH__LOGANALYZERS_REPORTBLOB_H

#include "xstd/String.h"
#include "xml/XmlTag.h"

// an XML container with a key or name
// a labeled "box" so others to find its contents
class ReportBlob: public XmlTag {
	public:
		typedef String Key;
		static const String NilTitle;

	public:
		ReportBlob(const Key &aKey, const String &aTitle);

		virtual XmlNode *clone() const;

		const Key &key() const { return theKey; }

	protected:
		Key theKey;
};

#endif
