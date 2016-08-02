
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_OBJID_H
#define POLYGRAPH__BASE_OBJID_H

#include "xstd/String.h"
#include "xstd/NetAddr.h"
#include "base/UniqId.h"

// a set of attributes that uniquely identifies a Polygraph object
class ObjId {
	protected:
		typedef enum { flgNone = 0, flgRepeat = 1, flgForeignSrc = 2,
			flgCachable = 4, flgHit = 8, flgUnused16 = 16, flgReload = 32,
			flgHot = 64, flgRediredReq = 128, flgRepToRedir = 256,
			flgMethodPost = 512, flgMethodHead = 1024, flgMethodPut = 2048,
			flgMethodGet = 4096, flgAborted = 8192,
			flgIms200 = 2*8192, flgIms304 = 4*8192,
			flgGzipContent = 8*8192, flgRange = 1<<17,
			flgAuthCred = 1<<18,
			flgSecure = 1<<19, // whether the scheme is secure (e.g., http becomes https)
			flgUpload = 1<<20,
			flgActive = 1<<21, // whether FTP transaction is active
			flgPassive = 1<<22, // whether FTP transaction is passive
			flgConnect = 1<<23 // whether transaction is HTTP CONNECT
		} Flag;

	public:
		ObjId() { reset(); }

		void reset();

		const UniqId &world() const { return theWorld; }
		int viserv() const { return theViserv; }
		int target() const { return theTarget; }
		int type() const { return theType; }
		Counter name() const { return theName; }
		int scheme() const { return theScheme; }
		const String &foreignUrl() const { return theForeignUrl; }

		bool direct() const;
		bool embedded() const;
		bool repeat() const { return (theFlags & flgRepeat) != 0; }
		bool hot() const { return (theFlags & flgHot) != 0; }
		bool foreignSrc() const { return (theFlags & flgForeignSrc) != 0; }

		bool basic() const { return get() && !(imsAny() || reload() || range() || rediredReq() || repToRedir() || aborted() || active() || passive() || connect()); }
		bool ims200() const { return (theFlags & flgIms200) != 0; }
		bool ims304() const { return (theFlags & flgIms304) != 0; }
		bool imsAny() const { return (theFlags & (flgIms200|flgIms304)) != 0; }
		bool reload() const { return (theFlags & flgReload) != 0; }
		bool range() const { return (theFlags & (flgRange)) != 0; }
		bool upload() const { return (theFlags & (flgUpload)) != 0; }
		bool rediredReq() const { return (theFlags & flgRediredReq) != 0; }
		bool repToRedir() const { return (theFlags & flgRepToRedir) != 0; }
		bool aborted() const { return (theFlags & flgAborted) != 0; }

		bool get() const { return (theFlags & flgMethodGet) != 0; }
		bool head() const { return (theFlags & flgMethodHead) != 0; }
		bool post() const { return (theFlags & flgMethodPost) != 0; }
		bool put() const { return (theFlags & flgMethodPut) != 0; }
		
		bool offeredHit() const { return basic() && repeat() && cachable(); }
		bool hit() const { return (theFlags & flgHit) != 0; }
		bool cachable() const { return (theFlags & flgCachable) != 0; }
		bool fill() const;

		bool gzipContent() const { return (theFlags & flgGzipContent) != 0; }
		bool authCred() const { return (theFlags & (flgAuthCred)) != 0; }
		bool secure() const { return (theFlags & (flgSecure)) != 0; }
		bool active() const { return (theFlags & (flgActive)) != 0; }
		bool passive() const { return (theFlags & (flgPassive)) != 0; }
		bool connect() const { return (theFlags & (flgConnect)) != 0; }

		void repeat(bool be) { setFlag(flgRepeat, be); }
		void hot(bool be) { setFlag(flgHot, be); }
		void foreignSrc(bool be) { setFlag(flgForeignSrc, be); }
		void foreignUrl(const String &aUrl) { theForeignUrl = aUrl; }

		void ims200(bool be) { setFlag(flgIms200, be); }
		void ims304(bool be) { setFlag(flgIms304, be); }
		void reload(bool be) { setFlag(flgReload, be); }
		void range(bool be) { setFlag(flgRange, be); }
		void upload(bool be) { setFlag(flgUpload, be); }
		void rediredReq(bool be) { setFlag(flgRediredReq, be); }
		void repToRedir(bool be) { setFlag(flgRepToRedir, be); }
		void aborted(bool be) { setFlag(flgAborted, be); }

		void get(bool be) { setFlag(flgMethodGet, be); }
		void head(bool be) { setFlag(flgMethodHead, be); }
		void post(bool be) { setFlag(flgMethodPost, be); }
		void put(bool be) { setFlag(flgMethodPut, be); }
		
		void hit(bool be) { setFlag(flgHit, be); }
		void cachable(bool be) { setFlag(flgCachable, be); }

		void gzipContent(bool be) { setFlag(flgGzipContent, be); }
		void authCred(bool be) { setFlag(flgAuthCred, be); }
		void secure(bool be) { setFlag(flgSecure, be); }
		void active(bool be) { setFlag(flgActive, be); }
		void passive(bool be) { setFlag(flgPassive, be); }
		void connect(bool be) { setFlag(flgConnect, be); }

		operator void*() const { return (theName > 0 || foreignUrl()) ? (void*)-1 : 0; }

		void world(const UniqId &aWorld) { theWorld = aWorld; }
		void viserv(int aViserv) { theViserv = aViserv; }
		void target(int aTarget) { theTarget = aTarget; }
		void type(int aType) { theType = aType; }
		void name(const Counter aName) { theName = aName; }
		void scheme(int aScheme) { theScheme = aScheme; }

		UniqId &world() { return theWorld; }
		Counter name() { return theName; }
		int type() { return theType; }

		int hash() const;

		inline bool operator <(const ObjId &id) const;  // arbitrary order
		inline bool operator ==(const ObjId &id) const;
		inline bool operator !=(const ObjId &id) const;

		ostream &printFlags(ostream &os) const;

		void store(OLog &ol) const;
		void load(ILog &il);

		Counter prefix() const { return thePrefix; }
		void prefix(const Counter aPrefix) { thePrefix = aPrefix; }
		bool calcPublic(const double prob);

		bool parse(const char *&buf, const char *end);

	protected:
		inline void setFlag(int flag, bool set);

	protected:
		String theForeignUrl;  // for foreign URLs only

		UniqId theWorld;   // world id
		int theViserv;     // visible server (index to Viservs)
		int theTarget;     // actual server (index to HostMap)
		Counter theName;   // object "name"
		int theType;       // object content type
		int theScheme;     // protocol scheme (Agent::Protocol)

		Counter thePrefix; // dynamic DNS prefix

		unsigned theFlags;
};


inline
bool ObjId::operator <(const ObjId &id) const {
	if (theName < id.theName)
		return true;
	if (theName > id.theName)
		return false;

	if (theViserv < id.theViserv)
		return true;
	if (theViserv > id.theViserv)
		return false;
	
	return theWorld < id.theWorld;
}

inline
bool ObjId::operator ==(const ObjId &id) const {
	return 
		theName == id.theName &&
		theViserv == id.theViserv &&
		theWorld == id.theWorld;
}

inline
bool ObjId::operator !=(const ObjId &id) const {
	return !(*this == id);
}

inline
void ObjId::setFlag(int flag, bool set) {
	if (set)
		theFlags |= flag;
	else
		theFlags &= ~flag;
}

inline
OLog &operator <<(OLog &ol, const ObjId &r) {
	r.store(ol);
	return ol;
}

inline
ILog &operator >>(ILog &il, ObjId &r) {
	r.load(il);
	return il;
}

#endif
