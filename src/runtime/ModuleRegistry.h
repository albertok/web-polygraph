
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_MODULEREGISTRY_H
#define POLYGRAPH__RUNTIME_MODULEREGISTRY_H

#include "xstd/h/iostream.h"
#include "xstd/Array.h"
#include "xstd/String.h"
#include "xstd/LibInit.h"

// common interface of various module registries
template <class ModuleType>
class ModuleRegistry {
	public:
		typedef ModuleType Module;

	public:
		inline ModuleRegistry(const String &aKind);
		inline ~ModuleRegistry(); // calls clear()

		inline void clear(); // deletes registered modules

		// registers, absorbs, and returns a unique module index
		int add(Module *m);

		void report(ostream &os); // reports registered modules

	protected:
		Array<Module*> theModules;
		String theKind;
};


template <class ModuleType>
inline
ModuleRegistry<ModuleType>::ModuleRegistry(const String &aKind): theKind(aKind) {
}

template <class ModuleType>
inline
ModuleRegistry<ModuleType>::~ModuleRegistry() {
	clear();
}

template <class ModuleType>
inline
void ModuleRegistry<ModuleType>::clear() {
	while (theModules.count())
		delete theModules.pop();
}

template <class ModuleType>
inline
int ModuleRegistry<ModuleType>::add(Module *m) {
	theModules.append(m);
	return theModules.count() - 1;
}

template <class ModuleType>
inline
void ModuleRegistry<ModuleType>::report(ostream &os) {
	os << "registered " << theKind << ": " << theModules.count();
	for (int i = 0; i < theModules.count(); ++i) {
		os << "\n\t" << theModules[i]->id() << ":\t ";
		theModules[i]->describe(os);
		os << endl;
	}
}

#endif
