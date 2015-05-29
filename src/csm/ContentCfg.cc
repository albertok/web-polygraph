
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"
#include "xstd/ZLib.h"
#include <fstream>

#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "base/RndPermut.h"
#include "base/BStream.h"
#include "base/rndDistrStat.h"
#include "base/ContTypeStat.h"
#include "base/polyLogCats.h"

#include "runtime/IOBuf.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "runtime/httpHdrs.h"
#include "runtime/httpText.h"
#include "pgl/MimeSym.h"
#include "pgl/ContentSym.h"
#include "csm/EmbedContMdl.h"
#include "csm/RndBodyIter.h"
#include "csm/CdbBodyIter.h"
#include "csm/ContainerBodyIter.h"
#include "csm/InjectIter.h"
#include "csm/GzipEncoder.h"
#include "csm/RangeBodyIter.h"
#include "csm/cdbEntries.h"
#include "csm/ContentDbase.h"
#include "csm/TextDbase.h"
#include "csm/ObjLifeCycle.h"
#include "csm/ContentCfg.h"

static const String DefaultContentCfgKind = "some-content";

int TheForeignContentId = -1;
int TheBodilessContentId = -1;
int TheUnknownContentId = -1;


ContentCfg::ContentCfg(int anId):
	theObjLifeCycle(0), theChbRatio(-1), theChecksumRatio(-1),
	theUniqueRatio(-1), theSize(0),
	theEmbedCont(0), theCdb(0),
	theTdb(0), theInjGap(0), theInfProb(-1),
	theExtSel(0), thePfxSel(0),
	theNewPerOid(0), theId(anId), 
	theEncodings(new int[codingEnd]),
	theClientBehaviorSym(0) {
	theObjLifeCycle = new ObjLifeCycle;
}

ContentCfg::~ContentCfg() {
	delete theObjLifeCycle;
	delete[] theEncodings;
}

void ContentCfg::configure(const ContentSym &cfg) {
	theKind = cfg.kind();
	if (!theKind)
		theKind = DefaultContentCfgKind;

	ContTypeStat::RecordKind(id(), kind());

	if (MimeSym *mime = cfg.mime()) {
		theMimeType = mime->mimeType();
		mime->extensions(theExtensions, theExtSel);
		mime->prefixes(thePrefixes, thePfxSel);
	}

	theSize = cfg.size();	
	theObjLifeCycle->configure(cfg.objLifeCycle());
	cfg.cachable(theChbRatio);
	cfg.checksum(theChecksumRatio);

	if (cfg.unique(theUniqueRatio) && theUniqueRatio >= 0) {
		// form unique prefix to be used to make some content unique
		const int sharedId = GlbPermut(rndSharedContent);
		char buf[64];
		ofixedstream os(buf, sizeof(buf));
        os << 'u' << hex << setfill('0') << 
			setw(8) << sharedId << '.' <<
			setw(2) << theId << '/' << ends;
		os.flush();
		theCommonPrefix = buf;
		const double commonRatio = 1 - theUniqueRatio;
		Comment(1) << "fyi: " << (100*commonRatio) << "% of '" <<
			theKind << "' content (id " << theId <<
			") will be identical; common prefix: " << theCommonPrefix <<
			endc;
	}
		
	if (cfg.hasEmbed()) {
		theEmbedCont = new EmbedContMdl;
		theEmbedCont->configure(&cfg);
	}

	if (const String &cdbName = cfg.cdb()) {
		ifstream f(cdbName.cstr());
		IBStream is;
		is.configure(&f, cdbName);
		theCdb = new ContentDbase;
		theCdb->load(is);
		if (!is.good())
			Comment << "error: cannot load content database from '"
				<< cdbName << "'; " << Error::Last() << endc << xexit;
		if (!theCdb->count())
			Comment << "error: no entries in '" << cdbName 
				<< "' content database" << endc << xexit;
	}

	//if (theEmbedCont && theCdb)
	//	Comment << "error: content cfg `" << theKind << "': "
	//		<< "cannot support containers together with content_db yet; "
	//		<< "do not use may_contain with content_db"
	//		<< endc << xexit;

	if (!theSize && !theCdb)
		Comment << "error: content cfg `" << theKind << "': "
			<< "has neither size distribution nor content_db; "
			<< "either one or both must be specified for "
			<< "Polygaph to know what object sizes this content type "
			<< "should generate"
			<< endc << xexit;

	if (const String &tdbName = cfg.injectDb()) {
		ifstream f(tdbName.cstr());
		theTdb = new TextDbase;
		theTdb->load(f);
		if (f.bad())
			Comment << "error: cannot load text database from `"
				<< tdbName << "; " << Error::Last() << endc << xexit;
		if (!theTdb->count())
			Comment << "error: text database `"
				<< tdbName << " appears to be empty" << endc << xexit;
	}

	theInjGap = cfg.injectGap();

	// XXX: we should put all inject* fields into one PGL object
	if (cfg.infectProb(theInfProb) != (theTdb != 0) || 
		(theTdb != 0) != (theInjGap != 0)) {
		Comment << "error: content cfg `" << theKind << "': "
			<< "`infect_prob' requires `inject_db' requires `inject_gap'"
			<< " and vice versa" 
			<< endc << xexit;
	}

	configureEncodings(cfg);

	theClientBehaviorSym = cfg.clientBehavior();
}

void ContentCfg::configureEncodings(const ContentSym &cfg) {
	theEncodings[codingIdentity] = theEncodings[codingGzip] = -1;

	Strings encodings;
	if (cfg.encodings(encodings)) {
		for (int i = 0; i < encodings.count(); ++i) {
			const String &encoding = *encodings[i];
			if (encoding == "identity")
				theEncodings[codingIdentity] = 0;
			else
			if (encoding == "gzip") {
				if (Deflator::Supported) {
					theEncodings[codingGzip] = 6;
				} else {
					Comment << "error: support for 'gzip' content encoding " <<
						"(fount in content cfg '" << theKind << "') has been " <<
						"disabled" << endc << xexit;
				}
			} else
				Comment << "error: unknown content encoding '" << encoding <<
					"' in content cfg '" << theKind << "'; known codings are " <<
					"'identity' and 'gzip'" << endc << xexit;
		}
	} else {
		theEncodings[codingIdentity] = 0;
	}
}

const String &ContentCfg::url_ext(int seed) const {
	return pickStr(theExtensions, theExtSel, seed);
}

const String &ContentCfg::url_pfx(int seed) const {
	return pickStr(thePrefixes, thePfxSel, seed);
}

double ContentCfg::repSizeMean() const {
	Assert(theSize || theCdb);
	if (theSize)
		return RndDistrStat(theSize).mean();
	else
		return theCdb->entrySizeMean();
}

bool ContentCfg::multipleContentCodings() const {
	return theEncodings[codingIdentity] >= 0 && theEncodings[codingGzip] >= 0;
}

void ContentCfg::calcTimes(const ObjId &oid, ObjTimes &times) const {
	const int seed = GlbPermut(oid.hash(), rndRepOlc);
	theObjLifeCycle->calcTimes(seed, times);
}

bool ContentCfg::calcContentCoding(ObjId &oid, const ReqHdr &req) const {
	if (theEncodings[codingGzip] >= 0 && req.acceptedEncoding(codingGzip))
		oid.gzipContent(true);
	else
	if (theEncodings[codingIdentity] >= 0 && req.acceptedEncoding(codingIdentity))
		oid.gzipContent(false);
	else
		return false;
	return true;
}

Size ContentCfg::calcRawRepSize(const ObjId &oid) const {
	Assert(theSize || theCdb);
	if (theSize) {
		const int seed = GlbPermut(contentHash(oid), rndRepSize);
		theSize->rndGen()->seed(seed);
		const double dh = theSize->trial();
		// prevent int overflows and leave room for headers
		// make sure uniquePrefix fits
		const Size contentPrefixSize = calcContentPrefixSize(oid);
		Size sz = (int)MiniMax((double)contentPrefixSize, 
			ceil(dh), (double)INT_MAX - 100*1024);
		// paranoid sanity checks
		if (!Should(sz >= contentPrefixSize))
			sz = contentPrefixSize;
		if (!Should(sz >= 0))
			sz = 0;
		return sz;
	} else {
		const int start = selectCdbStart(oid);
		CdbEntryPrnOpt opt;
		// assume that buf, injector, and rng are not needed
		opt.embed.model = theEmbedCont;
		opt.embed.container = oid;
		opt.sizeMax = Size(INT_MAX); // Size::Max();
		opt.entryOff = 0;
		return theCdb->entry(start)->size(opt);
	}
}

Size ContentCfg::calcFullEntitySize(const ObjId &oid) {
	BodyIter &i = *getBodyIter(oid);
	const Size res = i.fullEntitySize();
	i.putBack();
	return res;
}

bool ContentCfg::calcCachability(const ObjId &oid) const {
	const int seed = GlbPermut(oid.hash(), rndRepCach);
	RndGen rng(seed);
	return rng.event(theChbRatio);
}

bool ContentCfg::calcChecksumNeed(const ObjId &oid) const {
	const int seed = GlbPermut(oid.hash(), rndRepCheckNeed);
	RndGen rng(seed);
	return rng.event(theChecksumRatio);
}

xstd::Checksum ContentCfg::calcChecksum(const ObjId &oid) {
	WrBuf buf;
	xstd::ChecksumAlg checkAlg;
	BodyIter &i = *getBodyIter(oid);
	i.start(&buf);
	while (i) {
		i.pour();
		checkAlg.update(buf.content(), buf.contSize());
		buf.reset();
	}
	i.putBack();
	checkAlg.final();

	return checkAlg.sum();
}

Size ContentCfg::calcContentPrefixSize(const ObjId &oid) const {
	switch (contentUniqueness(oid)) {
		case cuUnique: {
			IOBuf buf;
			return pourUniqueContentPrefix(oid, buf);
		}
		case cuCommon:
			return theCommonPrefix.len();

		case cuChance:
		default:
			return 0;
	}
}

Size ContentCfg::pourContentPrefix(const ObjId &oid, IOBuf &buf) const {
	switch (contentUniqueness(oid)) {
		case cuUnique:
			return pourUniqueContentPrefix(oid, buf);

		case cuCommon: {
			buf.append(theCommonPrefix.data(), theCommonPrefix.len());
			return theCommonPrefix.len();
		}
		case cuChance:
		default:
			return 0;
	}
}

int ContentCfg::contentUniqueness(const ObjId &oid) const {
	if (theUniqueRatio < 0)
		return cuChance; // default: leave it to chance or other factors

	// no sense in generating content [prefix] for foreign oids
	if (!Should(!oid.foreignUrl() && !oid.foreignSrc()))
		return cuChance;
	
	const int seed = GlbPermut(oid.hash(), rndUniqueContent);
	RndGen rng(seed);
	return rng.event(theUniqueRatio) ? cuUnique : cuCommon;
}

// internal method, should be called only if uniqueContent()
Size ContentCfg::pourUniqueContentPrefix(const ObjId &oid, IOBuf &buf) const {
	// mimic Oid2Url() but do not use TheViservs and such, just indeces
	ofixedstream os(buf.space(), buf.spaceSize());
	os << 'u' << hex << setfill('0') <<
		'v' << setw(3) << oid.viserv() << '/' <<
		'w' << oid.world() << '/' <<
		't' << setw(2) << oid.type() << '/' <<
		'_' << setw(8) << ' ';
	os.flush();
	const Size size = (std::streamoff)os.tellp();
	Should(size < buf.spaceSize()); // otherwise may be too big
	buf.appended(size);
	return size;
}

int ContentCfg::contentHash(const ObjId &oid) const {
	if (contentUniqueness(oid) == cuCommon)
		return theId;
	else
		return oid.hash();
}

int ContentCfg::selectCdbStart(const ObjId &oid) const {
	Assert(theCdb);
	RndGen rng(GlbPermut(contentHash(oid), rndCdbStart));
	return rng(0, theCdb->count());
}

const String &ContentCfg::pickStr(const Strings &strings, RndDistr *sel, int seed) const {
	static String noStr = 0;
	if (const int count = strings.count()) {
		sel->rndGen()->seed(seed);
		const int idx = (int)sel->trial();
		Assert(0 <= idx && idx < count);
		return *strings[idx];
	}
	return noStr;
}

double ContentCfg::compContPerCall(const ContentCfg *cc) const {
	if (cc->id() == id())
		return 1.0;

	if (theEmbedCont)
		return theEmbedCont->compContPerCall(cc);

	return 0.0;
}

void ContentCfg::noteNewContProb(ContentCfg *cc, double newProb) {
	if (cc->id() == id())
		return;

	if (theEmbedCont)
		theEmbedCont->noteNewContProb(cc, newProb);
}

void ContentCfg::newPerOid(double aNewPerOid) {
	Assert(aNewPerOid > 0);
	theNewPerOid = aNewPerOid;
}

// XXX: iterators should be farmed, but it is hard because they
// come in different types (perhaps somebody else should farm them?)
BodyIter *ContentCfg::getBodyIter(const ObjId &oid, const RangeList *const ranges) {
	BodyIter *res = 0;

	if (theCdb) {
		CdbBodyIter *i = new CdbBodyIter;
		i->cdb(theCdb);
		if (theEmbedCont)
			i->embedContModel(theEmbedCont);
		i->startPos(selectCdbStart(oid));

		if (theTdb) {
			RndGen rng(GlbPermut(contentHash(oid), rndInjProb));
			// should we inject this object?
			if (rng.event(theInfProb)) {
				InjectIter *inj = new InjectIter; // XXX: Farm these!
				inj->creator(this);
				inj->textDbase(theTdb);
				inj->gap(theInjGap);
				i->injector(inj);
			}
		}

		res = i;
	} else
	if (theEmbedCont) {
		ContainerBodyIter *i = new ContainerBodyIter;
		i->embedContModel(theEmbedCont);
		res = i;
	} else {
		RndBodyIter *i = new RndBodyIter;
		res = i;
	}	

	if (Should(res)) {
		// keep in sync with GzipEncoder ctor
		res->contentCfg(this);
		res->oidCfg(oid, contentHash(oid));
		res->contentSize(calcRawRepSize(oid));
		if (oid.gzipContent())
			res = new GzipEncoder(theEncodings[codingGzip], res);
		if (oid.range() && ranges)
			res = new RangeBodyIter(*ranges, res);
	}

	return res;
}

void ContentCfg::putBodyIter(BodyIter *i) const {
	i->stop();
	delete i;
}

void ContentCfg::putInjector(InjectIter *i) const {
	delete i; // XXX: Farm these?
}
