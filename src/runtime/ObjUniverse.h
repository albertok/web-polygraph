
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_OBJUNIVERSE_H
#define POLYGRAPH__RUNTIME_OBJUNIVERSE_H

#include "runtime/ObjGalaxy.h"

class OBStream;
class IBStream;
class HostCfg;

// All visible server oids grouped by galaxies.
// Not an ObjWorld child, but mimics its major interfaces.
class ObjUniverse {
	public:
		typedef std::vector<ObjGalaxy> Galaxies;

	protected:
		typedef bool (ObjWorld::*WorldKind)() const;

	public:
		// absorbs universe, may return different (old) universe
		static ObjUniverse *Add(HostCfg *host, ObjUniverse *universe);
		// replaced old universe if needed
		static ObjUniverse *Put(HostCfg *host, ObjUniverse *universe);
		static int Count();

		// scan all universes
		static bool Frozen();
		static void FreezeWss();
		static Counter CurrentWss(int &frozenCount, int &totalCount);
		static void DumpGalaxies(ostream &os);
		static void ReportWss(int commentLvl);

	public:
		ObjUniverse();
		ObjUniverse(const UniqId &localId);

		bool find(const UniqId &id, int &galaxyIdx) const;

		// "single use" pointer or nil
		const ObjWorld *worldToSync() const;
		const ObjWorld *localWorldToSync() const;
		const ObjWorld *newerWorld(const ObjWorld &oldWorld, int startIdx = 0) const;

		bool canRepeat(const int type) const;
		bool canRepeatLocal(const int type) const;
		bool canProduce(const int type) const;
		Counter size() const;

		void repeat(ObjId &oid, ObjSelector *sel);
		void repeatLocal(ObjId &oid, ObjSelector *sel);
		void produce(ObjId &oid, RndGen &rng);
		void freezeWss();

		void update(const ObjWorld &world);

		void store(OBStream &os) const;
		void load(IBStream &is);

		void dumpGalaxies(ostream &os) const;

	protected:
		Counter currentWss(int &frozenCount, int &totalCount) const;

		void add(const ObjWorld &world);

		ObjGalaxy &getGalaxy(const WorldKind kind, const int type);

		// the galaxy for this polyclt process
		// fail if no requested galaxy
		const ObjGalaxy &localGalaxy() const;
		ObjGalaxy &localGalaxy();

		const ObjGalaxy &galaxyAt(const Galaxies::size_type idx) const;
		ObjGalaxy &galaxyAt(const Galaxies::size_type idx);

		const ObjWorld &world(const unsigned idx) const;

	protected:
		static int TheCount; // total number of universes Add()ed

		Galaxies theGalaxies; // galaxies in no particular order
		int theLocalGalaxy; // const except during ctor/loading
		// index in concatenated array of all worlds from all galaxies
		mutable unsigned int theNextWorldToSync;
		// world index in the local galaxy
		mutable unsigned int theNextLocalWorldToSync;
};


inline
OBStream &operator <<(OBStream &os, const ObjUniverse &universe) {
	universe.store(os);
	return os;
}

inline
IBStream &operator >>(IBStream &is, ObjUniverse &universe) {
	universe.load(is);
	return is;
}

#endif
