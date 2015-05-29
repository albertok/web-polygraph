
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_CONTENTCFG_H
#define POLYGRAPH__CSM_CONTENTCFG_H

#include "xstd/String.h"
#include "xstd/Array.h"
#include "runtime/httpHdrs.h"

class ClientBehaviorSym;
class ObjId;
class RndDistr;
class BodyIter;
class ContentDbase;
class TextDbase;
class ContentSym;
class EmbedContMdl;
class ObjTimes;
class ObjLifeCycle;
class InjectIter;
class IOBuf;

class ContentCfg {
	public:
		typedef Array<String*> Strings;

	public:
		ContentCfg(int anId);
		~ContentCfg();

		void configure(const ContentSym &cfg);

		int id() const { return theId; }
		const String &kind() const { return theKind; }
		double newPerOid() const { return theNewPerOid; }
		const String &url_ext(int seed) const;
		const String &url_pfx(int seed) const;
		double repSizeMean() const;
		bool hasEmbedCont() const { return theEmbedCont != 0; }
		bool multipleContentCodings() const;
		const ClientBehaviorSym *clientBehaviorSym() const { return theClientBehaviorSym; }

		void calcTimes(const ObjId &oid, ObjTimes &times) const;
		bool calcContentCoding(ObjId &oid, const ReqHdr &req) const;
		Size calcRawRepSize(const ObjId &oid) const;
		Size calcFullEntitySize(const ObjId &oid);
		bool calcCachability(const ObjId &oid) const;
		bool calcChecksumNeed(const ObjId &oid) const;
		xstd::Checksum calcChecksum(const ObjId &oid);
		Size pourContentPrefix(const ObjId &oid, IOBuf &buf) const;

		double compContPerCall(const ContentCfg *cc) const;
		void noteNewContProb(ContentCfg *cc, double newProb);
		void newPerOid(double aNewPerOid);

		BodyIter *getBodyIter(const ObjId &oid, const RangeList *const ranges = 0);
		void putBodyIter(BodyIter *i) const;
		void putInjector(InjectIter *i) const;

	protected:
		void configureEncodings(const ContentSym &cfg);

		int selectCdbStart(const ObjId &oid) const;
		const String &pickStr(const Strings &strings, RndDistr *sel, int seed) const;

		Size calcContentPrefixSize(const ObjId &oid) const;
		Size pourUniqueContentPrefix(const ObjId &oid, IOBuf &buf) const;
		int contentUniqueness(const ObjId &oid) const;
		int contentHash(const ObjId &oid) const;

	public:
		String theKind;
		String theMimeType;
		ObjLifeCycle *theObjLifeCycle;
		double theChbRatio;
		double theChecksumRatio;
		double theUniqueRatio;

	protected:
		RndDistr *theSize;        // null if must use cdb entry sizes
		EmbedContMdl *theEmbedCont;
		ContentDbase *theCdb;

		TextDbase *theTdb;
		RndDistr *theInjGap;      // distance between injections
		double theInfProb;        // portion of infected files

		Strings theExtensions;       // file extensions
		mutable RndDistr *theExtSel; // selector for the above

		Strings thePrefixes;         // URL path prefixes
		mutable RndDistr *thePfxSel; // selector for the above

		double theNewPerOid; // new content oids per direct access oid
		int theId;
		String theCommonPrefix;

		int *theEncodings;   // content-encodings

		const ClientBehaviorSym *theClientBehaviorSym; // client behavior sym

		enum ContentUniqueness { cuChance = 0, cuUnique, cuCommon };
};

extern int TheForeignContentId;
extern int TheBodilessContentId;
extern int TheUnknownContentId;

#endif
