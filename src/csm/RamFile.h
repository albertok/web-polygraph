
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_RAMFILE_H
#define POLYGRAPH__CSM_RAMFILE_H

#include "xstd/String.h"

// a representation of a disk file in RAM
class RamFile {
	public:
		String name; // including path, relative to the document_root
		String body; // all raw content bytes
};

#endif /* POLYGRAPH__CSM_RAMFILE_H */
