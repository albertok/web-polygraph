
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_OBJGALAXY_H
#define POLYGRAPH__RUNTIME_OBJGALAXY_H

#include <vector>
#include "runtime/ObjWorld.h"

class OBStream;
class IBStream;

// Contains one ObjWorld for each configured content type.
// All stored ObjWolrds have the same world ID.
class ObjGalaxy {
	public:
		ObjGalaxy() {}

		const UniqId &id() const;
		void id(const UniqId &id);

		Counter currentWss(bool &isFrozen) const;
		Counter size() const;
		void freezeWss();

		void store(OBStream &os) const;
		void load(IBStream &is);
		void dumpWorlds(ostream &os) const;
		void update(const ObjWorld &newWorld);

		const ObjWorld &world(const int type) const;
		ObjWorld &world(const int type);

		static int Capacity();

	private:
		typedef std::vector<ObjWorld> ObjWorlds;
		ObjWorlds theObjWorlds; // worlds indexed by content type ID
};

inline
OBStream &operator <<(OBStream &os, const ObjGalaxy &galaxy) {
	galaxy.store(os);
	return os;
}

inline
IBStream &operator >>(IBStream &is, ObjGalaxy &galaxy) {
	galaxy.load(is);
	return is;
}

#endif
