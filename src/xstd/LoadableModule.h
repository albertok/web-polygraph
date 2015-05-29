
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_LOADABLEMODULE_H
#define POLYGRAPH__XSTD_LOADABLEMODULE_H

#include "xstd/String.h"

// wrapper for dlopen(3) and friends
class LoadableModule {
	public:
		enum LoadMode { lmNow, lmLazy };

	public:
		LoadableModule(const String &aName);
		~LoadableModule();                   // unloads if loaded

		bool loaded() const;
		const String &name() const { return theName; }
		const String &error() const { return theError; }

		bool load(int mode = lmNow);
		bool unload();

	protected:
		String theName;
		String theError;
		void *theHandle;
};

#endif
