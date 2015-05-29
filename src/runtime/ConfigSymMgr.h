
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_CONFIGSYMMGR_H
#define POLYGRAPH__RUNTIME_CONFIGSYMMGR_H

#include "xstd/Array.h"

// prevents creation of large number of configuration objects based
// on identical PGL descriptions
template <class Sym, class Cfg>
class ConfigSymMgr {
	public:
		inline virtual ~ConfigSymMgr();

		inline int count() const { return theCfgs.count(); }
		inline bool validId(int id) const;

		inline Cfg *get(const Sym *cs);
		inline Cfg *get(int id);
		inline void get(const Array<Sym*> &syms, Array<Cfg*> &cfgs);

		inline Cfg *operator [](int id) { return get(id); }

	protected:
		virtual Cfg *makeCfg() = 0;
		
	protected:
		Array<Cfg*> theCfgs;
		Array<Sym*> theSyms; // kept to search for new symbols
};


template <class Sym, class Cfg>
inline
ConfigSymMgr<Sym, Cfg>::~ConfigSymMgr() {
	while (theCfgs.count()) delete theCfgs.pop();
	while (theSyms.count()) delete theSyms.pop();
}

// this method can be called re-entrantly!
template <class Sym, class Cfg>
inline
Cfg *ConfigSymMgr<Sym, Cfg>::get(const Sym *cs) {
	Assert(cs);

	int idx;
	for (idx = 0; idx < theSyms.count(); ++idx) {
		if (theSyms[idx] && cs->equal(*theSyms[idx]))
			return theCfgs[idx];
	}

	// no match found
	Cfg *cfg = makeCfg();
	theCfgs.append(cfg);
	theSyms.append((Sym*)cs->clone());

	// configure after append to facilitate reentrant calls
	cfg->configure(*cs);
	return cfg;
}

template <class Sym, class Cfg>
inline
bool ConfigSymMgr<Sym, Cfg>::validId(int id) const {
	const int idx = id - 1;
	return 0 <= idx && idx < theCfgs.count();
}

template <class Sym, class Cfg>
inline
Cfg *ConfigSymMgr<Sym, Cfg>::get(int id) {
	Assert(0 <= id && id < theCfgs.count());
	return theCfgs[id];
}

template <class Sym, class Cfg>
inline
void ConfigSymMgr<Sym, Cfg>::get(const Array<Sym*> &syms, Array<Cfg*> &cfgs) {
	for (int i = 0; i < syms.count(); ++i)
		cfgs.append(get(syms[i]));
}


#endif
