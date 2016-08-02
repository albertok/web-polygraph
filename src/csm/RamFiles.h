
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_RAMFILES_H
#define POLYGRAPH__CSM_RAMFILES_H

#include "base/polygraph.h"
#include "base/AggrStat.h"
#include "csm/RamFile.h"
#include "xstd/String.h"
#include "xstd/Size.h"
#include <vector>

// A RAM cache to store files loaded from disk.
class RamFiles {
	public:
		explicit RamFiles(const String &aPath): thePath(aPath) {}

		void load();
		int count() const { return theFiles.size(); }
		const RamFile &fileAt(const int pos) const { return theFiles.at(pos); }

		// total for all files cached in RAM
		BigSize ramSize() const { return theStats.sum(); }

		// average size of an individual cached file
		double fileSizeMean() const { return theStats.mean(); }

	private:
		const String thePath; // common path for all cached files
		AggrStat theStats;

		typedef std::vector<RamFile> Storage;
		Storage theFiles;

};

#endif
