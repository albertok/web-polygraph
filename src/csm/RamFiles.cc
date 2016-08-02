
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "csm/RamFiles.h"
#include "runtime/LogComment.h"
#include "xstd/DirScanner.h"
#include <fstream>
#include <limits>

void RamFiles::load() {
	int badFileCount = 0;
	int hugeFileCount = 0;
	int emptyFileCount = 0;

	DirScanner scanner(thePath);
	String fullFileName;
	while (scanner.getNext(fullFileName)) {
		ifstream is(fullFileName.cstr(), std::ios::binary);
		if (!is) {
			++badFileCount;
			Comment(0) << "error: cannot open file " << fullFileName << ": " << Error::Last() << endc;
			continue;
		}

		is.seekg(0, std::ios_base::end);
		const streampos fileSize = is.tellg();
		is.seekg(0, std::ios_base::beg);
		if (!is || fileSize < 0) {
			++badFileCount;
			Comment(0) << "error: cannot estimate " << fullFileName << " size: " << Error::Last() << endc;
			continue;
		}

		// Skip empty files for now because it is difficult to
		// handle them when Content.size is also configured.
		if (fileSize == 0) {
			if (!emptyFileCount) {
				Comment(1) << "warning: skipping empty files, " <<
					"starting with: " << fullFileName << endc;
			}
			emptyFileCount++;
			continue;
		}

		if (fileSize > std::numeric_limits<Size::size_type>::max()) {
			Comment(1) << "warning: skipping " << fullFileName << " which " <<
				"size (" << fileSize << " bytes) exceeds Polygraph internal " <<
				"limit of " << std::numeric_limits<Size::size_type>::max() << " bytes" << endc;
			++hugeFileCount;
			continue;
		}

		const size_t ramSize = static_cast<size_t>(fileSize);
		const streampos sizeCheck = static_cast<streampos>(ramSize);
		if (fileSize != sizeCheck) {
			Comment(1) << "warning: skipping " << fullFileName << " which " <<
				"size (" << fileSize << " bytes) exceeds Polygraph in-RAM " <<
				"object limit of " << std::numeric_limits<size_t>::max() << " bytes" << endc;
			++hugeFileCount;
			continue;
		}

		RamFile ramFile;
		is.read(ramFile.body.alloc(ramSize), ramSize);
		if (!is) {
			Comment(0) << "error: failure reading " << BigSize(ramSize) <<
				" from " << fullFileName << ": " << Error::Last() << endc;
			++badFileCount;
			continue;
		}
		ramFile.name = fullFileName.area(thePath.len()).data();
		theFiles.push_back(ramFile);
		theStats.record(ramSize);
	}

	if (badFileCount)
		Comment(3) << "fyi: failed to load " << badFileCount << " bad file(s) from " << thePath << endc;

	if (hugeFileCount)
		Comment(3) << "fyi: skipped " << hugeFileCount << " huge file(s) from " << thePath << endc;

	if (emptyFileCount)
		Comment(3) << "warning: skipped " << emptyFileCount << " empty file(s) from " << thePath << endc;

	Comment(3) << "fyi: loaded " << count() << " file(s) from " << thePath << endc;
}
