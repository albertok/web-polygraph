
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_SECTIONSTATE_H
#define POLYGRAPH__LOGANALYZERS_SECTIONSTATE_H

#include "xstd/String.h"
#include "xstd/Array.h"

class XmlTag;

class SectionState {
	public:
		SectionState();

		void reset();

		int level() const;

		String begSection(const XmlTag &s, String &trueNum); // suggests section "number"
		void endSection();

	protected:
		String curNum(const XmlTag &s, String &trueNum) const;

	protected:
		Array<int> theLevels;
};

#endif
