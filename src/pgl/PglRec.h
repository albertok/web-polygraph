
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLREC_H
#define POLYGRAPH__PGL_PGLREC_H

#include "xparser/SynSymTbl.h"

// holds "record"-like or "struct"-like info
// records can be nested
class PglRec: public SynSymTbl {
	public:
		PglRec();
		virtual ~PglRec();

		virtual SynSymTblItem **valp(const String &key);
		PglRec *clone() const;
		void copyCommon(const PglRec &rec);

		// builds a SynSymTblItem and adds it
		SynSymTblItem *bAdd(const String &type, const String &name, SynSym *s);
};

#endif
