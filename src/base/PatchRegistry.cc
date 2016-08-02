
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"
#include "xstd/h/string.h"
#include "base/PatchRegistry.h"

#include <list>
#include <algorithm>

// basic information about the patch
class Patch {
public:
	Patch(const char *anId, const char *aGist): id(anId), gist(aGist) {}

	bool operator ==(const Patch &p) const { return strcasecmp(id, p.id) == 0; }

	const char *id; // usually a file name without extension
	const char *gist; // what the patch does; a one-liner
};

typedef std::list<Patch> Patches;
static Patches *ThePatches = 0; // registered patches in application order


static
ostream &operator <<(ostream &os, const Patch &p) {
	return os << p.id << " -\t " << p.gist;
}


void ReportPatches(std::ostream &os) {
	if (!ThePatches)
		return;

	os << "Registered patches:" << std::endl;
	for (Patches::const_iterator i = ThePatches->begin(); i != ThePatches->end(); ++i)
		os << "patch: " << *i << std::endl;
}

int CountPatches() {
	return ThePatches ? ThePatches->size() : 0;
}

static
void RegisterPatch(const Patch &p) {
	if (!ThePatches)
		ThePatches = new Patches;

	const Patches::const_iterator other =
		std::find(ThePatches->begin(), ThePatches->end(), p);
	if (other != ThePatches->end()) {
		std::cerr << "Warning: Duplicate patches registered:" << std::endl <<
			"\tpatch1: " << p << std::endl <<
			"\tpatch2: " << *other << std::endl;
		// but register anyway to highlight the problem
	}

	// we assume that patches call us in reversed application order
	ThePatches->push_front(p);
}

// A hack to keep RegisterPatches() source line offset constant so that it is
// easier to patch that function using register-patch.pl.
#include "base/PatchRegistrations.h"

/// convenience function to "use" an otherwise unreferenced static names
template <class Var>
static volatile
bool UseThisStatic(const Var &var) {
	return sizeof(var) > 0;
}

/// convenience hack to call RegisterPatches() automatically
static const bool Register_ = RegisterPatches() &&
	UseThisStatic(&Register_) && UseThisStatic(&RegisterPatch);
