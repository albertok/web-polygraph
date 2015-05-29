
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_XMLTAGIDENTIFIER_H
#define POLYGRAPH__CSM_XMLTAGIDENTIFIER_H

#include "xstd/PrefixIdentifier.h"

// identifies tags (and their attributes) of interest
class XmlTagIdentifier {
	public:
		XmlTagIdentifier();
		~XmlTagIdentifier();

		void configure(Array<String*> &tags);

		int tagCount() const { return theTagIdx.count(); }

		bool findTag(const char *nameBeg, int nameLen, int &id) const;
		bool findAttr(int tagId, const char *nameBeg, int nameLen) const;

	protected:
		void addTag(const String &str);

	private:
		XmlTagIdentifier(const XmlTagIdentifier&);

	protected:
		PrefixIdentifier theTagIdx;
		Array<PrefixIdentifier*> theAttrIndeces;
};

#endif
