
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_RAMFILEBODYITER_H
#define POLYGRAPH__CSM_RAMFILEBODYITER_H

#include "csm/BodyIter.h"
#include "csm/RamFile.h"

// pours a RamFile
class RamFileBodyIter: public BodyIter {
	public:
		RamFileBodyIter();
		virtual RamFileBodyIter *clone() const;

		virtual void start(WrBuf *aBuf);

		void file(const RamFile &aFile);

	protected:
		virtual bool pourMiddle();
		virtual void calcContentSize() const;

	protected:
		RamFile theFile; // the source of the body content
		Size theOff; // bytes poured from theFile
};

#endif /* POLYGRAPH__CSM_RAMFILEBODYITER_H */
