
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_PUBWORLD_H
#define POLYGRAPH__RUNTIME_PUBWORLD_H

#include "xstd/Array.h"
#include "runtime/ObjWorld.h"

class OBStream;
class IBStream;
class HostCfg;

// per-polyclt public world slice for a given visible server
class PubWorldSlice: public ObjWorld {
	public:
		PubWorldSlice() {};
		PubWorldSlice(const ObjWorld &w): ObjWorld(w) {}

		void update(const ObjWorld &w);
};

// global public world for a given visible server
// not a ObjWorld child, but mimics its major interfaces
class PubWorld {
	protected:
		typedef bool (PubWorldSlice::*SliceKind)() const;

	public:
		// absorbs pubWorld, may return different (old) pubWorld
		static PubWorld *Add(HostCfg *host, PubWorld *pubWorld);
		// replaced old pubWorld if needed
		static PubWorld *Put(HostCfg *host, PubWorld *pubWorld);
		static int Count();

		// scan all public worlds
		static bool Frozen();
		static void FreezeWss();
		static int CurrentWss(int &frozenCount, int &totalCount); 
		static void DumpSlices(ostream &os);
		static void ReportWss(int commentLvl);

	public:
		PubWorld();
		PubWorld(const UniqId &localId);

		bool find(const UniqId &id, int &sliceIdx) const;

		// "single use" pointer or nil
		const PubWorldSlice *sliceToSync() const;
		const PubWorldSlice *newerSlice(const ObjWorld &oldSlice, int startIdx = 0) const;

		// fails if no requested slice
		const PubWorldSlice &localSlice() const;
		const PubWorldSlice &sliceAt(int idx) const;
		PubWorldSlice &sliceAt(int idx);

		bool canRepeat() const;
		bool canRepeatLocal() const;
		bool canProduce() const;
		int size() const;

		void repeat(ObjId &oid, ObjSelector *sel);
		void repeatLocal(ObjId &oid, ObjSelector *sel);
		void produce(ObjId &oid, RndGen &rng);
		void freezeWss();

		void add(const PubWorldSlice &slice);

		void store(OBStream &os) const;
		void load(IBStream &is);

		void dumpSlices(ostream &os) const;

	protected:
		int currentWss(int &frozenCount, int &totalCount) const;

		PubWorldSlice &getSlice(SliceKind kind);
		PubWorldSlice &localSlice(); // the slice for this polyclt process

	protected:
		static int TheCount; // total number of public worlds Add()ed

		Array<PubWorldSlice> theSlices; // all slices
		int theLocalSlice;              // const except during ctor/loading
		mutable int theNextSliceToSync;
};


inline
OBStream &operator <<(OBStream &os, const PubWorld &pw) {
	pw.store(os);
	return os;
}

inline
IBStream &operator >>(IBStream &is, PubWorld &pw) {
	pw.load(is);
	return is;
}

inline
OBStream &operator <<(OBStream &os, const PubWorldSlice &slice) {
	slice.store(os);
	return os;
}

inline
IBStream &operator >>(IBStream &is, PubWorldSlice &slice) {
	slice.load(is);
	return is;
}

#endif
