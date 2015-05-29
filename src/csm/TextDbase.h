
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_TEXTDBASE_H
#define POLYGRAPH__CSM_TEXTDBASE_H

#include "xstd/String.h"
#include "xstd/Array.h"


// a compact array of strings with pre-calculated lenghts

class TextDbase {
	public:
		TextDbase();
		~TextDbase();

		const String &name() const { return theName; }
		int count() const { return theStrs.count(); }
		int len(int pos) const { return theLens[pos]; }
		const char *str(int pos) const { return theStrs[pos]; }

		void name(const String &aName);

		void load(istream &is);

	protected:
		void add(const char *str, int len);

	protected:
		Array<char*> theStrs;
		Array<int> theLens;

		String theName;
};


#endif
