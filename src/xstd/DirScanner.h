
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_DIR_SCANNER_H
#define POLYGRAPH__XSTD_DIR_SCANNER_H

#include "xstd/String.h"
#include "xstd/h/sys/types.h"
#include <deque>
#if HAVE_DIRENT_H
#include <dirent.h>
#endif

// iterates directory tree to find file names (in semi-depth-first order)
// follows links up to a system limit
class DirScanner {
	public:
		explicit DirScanner(const String &topOfTheTreePath);
		bool getNext(String &path); // fills path when returning true

	private:
		typedef enum { etFile, etDir, etLinkDir, etOther, etErr } EntryType;

		bool openDir(const String &path, int symLevel);
		void addDebt(const String &fullPath, const int symLevel);
		EntryType type(struct dirent *entry, const String &fullPath) const;

		typedef std::pair<String, int> DirEntry; // dir entry name, link level
		typedef std::deque<DirEntry> Debt; // dir entries to scan
		Debt theDebt;

		typedef std::deque<String> FileNames; // in the order they are found
		FileNames theFileNames;
};

#endif
