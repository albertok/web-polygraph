
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"
#include "xstd/h/sys/stat.h"
#include "xstd/h/string.h"
#include "xstd/h/os_std.h"
#include "xstd/Assert.h"
#include "xstd/DirScanner.h"
#include <iostream>


#if defined(SYMLOOP_MAX)
static const int SysLinkLimit = SYMLOOP_MAX;
#elif defined(_SC_SYMLOOP_MAX)
static const int SysLinkLimit = sysconf(_SC_SYMLOOP_MAX);
#else
static const int SysLinkLimit = -1;
#endif
static const int LinkLimit = SysLinkLimit > 0 ? SysLinkLimit : 8;


DirScanner::DirScanner(const String &path) {
	switch (type(0, path)) {
		case etFile:
			theFileNames.push_back(path);
			break;
		case etDir:
			addDebt(path, 0);
			break;
		case etLinkDir:
			addDebt(path, 1);
			break;
		case etErr:
			std::cerr << "error: cannot stat(2) " << path << std::endl;
			break;
		case etOther:
			std::cerr << "error: expecting a directory or regular file: " << path << std::endl;
	}
}

bool DirScanner::getNext(String &fileName) {
	while (theFileNames.empty() && !theDebt.empty()) {
		// removing the last element and adding new dept at the end
		// approximates depth-first scanning order which minimizes
		// RAM usage for debt holding in a typical directories layout
		// (a few levels but potentially lots of files per level)
		const DirEntry entry = theDebt.back();
		theDebt.pop_back();
		openDir(entry.first, entry.second);
	}

	if (theFileNames.empty())
		return false;

	fileName = theFileNames.back();
	theFileNames.pop_back();
	return true;
}

bool DirScanner::openDir(const String &path, int symLevel) {
	DIR *dir = opendir(path.cstr());
	if (!dir) {
		std::cerr << "error: cannot open directory " << path << ": " << Error::Last() << std::endl;
		return false;
	}

	while (struct dirent *entry = readdir(dir)) {
		if (entry->d_name[0] == '.')
			continue;

		String fullPath(path + "/" + entry->d_name);
		switch (type(entry, fullPath)) {
			case etFile:
				theFileNames.push_back(fullPath);
				break;
			case etDir:
				addDebt(fullPath, symLevel);
				break;
			case etLinkDir:
				addDebt(fullPath, symLevel + 1);
				break;
			case etErr:
				std::cerr << "error: cannot stat(2): " << fullPath << std::endl;
				break;
			case etOther:
				break;
		}
	}

	closedir(dir);
	return true;
}

void DirScanner::addDebt(const String &fullPath, const int symLevel) {
	if (symLevel > LinkLimit)
		std::cerr << "error: symbolic link count exceeded " << LinkLimit << ": " << fullPath << std::endl;
	else
		theDebt.push_back(std::make_pair(fullPath, symLevel));
}

DirScanner::EntryType DirScanner::type(struct dirent *entry, const String &fullPath) const {
#ifdef _DIRENT_HAVE_D_TYPE
	if (entry) {
		if (entry->d_type == DT_DIR)
			return etDir;
		if (entry->d_type == DT_REG)
			return etFile;
		if (entry->d_type != DT_LNK)
			return etOther;
	} else
#endif
	{
		struct stat stbuf;
		if (!ShouldSys(lstat(fullPath.cstr(), &stbuf) >= 0))
			return etErr;
		if (S_ISDIR(stbuf.st_mode))
			return etDir;
		if (S_ISREG(stbuf.st_mode))
			return etFile;
		if (!S_ISLNK(stbuf.st_mode))
			return etOther;
	}

	/* handle symbolic links */
	struct stat stbuf;
	if (!ShouldSys(stat(fullPath.cstr(), &stbuf) >= 0))
		return etErr;
	if (S_ISDIR(stbuf.st_mode))
		return etLinkDir;
	if (S_ISREG(stbuf.st_mode))
		return etFile;
	return etOther;
}
