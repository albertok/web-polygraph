
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_STATTABLE_H
#define POLYGRAPH__LOGANALYZERS_STATTABLE_H

#include "base/polygraph.h"

#include "loganalyzers/Stex.h"
#include "xml/XmlTable.h"

#include <list>

class StatTable: public XmlTable {
	public:
		StatTable() {}
		explicit StatTable(const XmlTable &table): XmlTable(table) {}

		void addUnknown(const Stex &s) { if (!s.ignoreUnseen()) theUnknowns.push_back(s.name()); }
		const std::list<String> &unknowns() const { return theUnknowns; }

	protected:
		std::list<String> theUnknowns;
};

XmlTag &operator <<(XmlTag &tag, const StatTable &statTable);

#endif
