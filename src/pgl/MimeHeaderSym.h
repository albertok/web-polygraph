
/* Web Polygraph       http://www.web-polygraph.org/
 * (C) 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_MIMEHEADERSYM_H
#define POLYGRAPH__PGL_MIMEHEADERSYM_H

#include "xstd/String.h"
#include "pgl/PglRecSym.h"

class MimeHeaderSym: public RecSym {
	public:
		static const String TheType;

	public:
		MimeHeaderSym();
		MimeHeaderSym(const String &aType, PglRec *aRec);

		static MimeHeaderSym *Parse(const String &s);

		virtual bool isA(const String &type) const;

		const String *name() const;
		const String *value() const;
	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
