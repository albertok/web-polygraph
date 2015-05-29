
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/dlfcn.h"

#include "xstd/Assert.h"
#include "xstd/LoadableModule.h"

LoadableModule::LoadableModule(const String &aName): theName(aName), theHandle(0) {
}

LoadableModule::~LoadableModule() {
	if (loaded())
		unload();
}

bool LoadableModule::loaded() const {
	return theHandle != 0;
}

bool LoadableModule::load(int mode) {
	if (Should(!loaded())) {
		theHandle = dlopen(theName.cstr(),
			mode == lmNow ? RTLD_NOW : RTLD_LAZY);
		if (loaded())
			return true;
		theError = dlerror();
	} else {
		theError = "internal error: reusing LoadableModule object";
	}
	return false;
}

bool LoadableModule::unload() {
	if (Should(loaded())) {
		if (dlclose(theHandle) == 0)
			return true;
		theError = dlerror();
	} else {
		theError = "internal error: unloading not loaded module";
	}
	return false;
}
