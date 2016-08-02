
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
#include "xstd/StringIdentifier.h"
#include "xstd/gadgets.h"
#include "base/RndPermut.h"
#include "base/BStream.h"
#include "base/rndDistrStat.h"
#include "base/ContTypeStat.h"
#include "base/polyLogCats.h"

#include "runtime/IOBuf.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/MimeHeadersCfg.h"
#include "runtime/polyErrors.h"
#include "runtime/httpHdrs.h"
#include "runtime/httpText.h"
#include "pgl/MimeSym.h"
#include "pgl/ContentSym.h"
#include "csm/EmbedContMdl.h"
#include "csm/RndBodyIter.h"
#include "csm/CdbBodyIter.h"
#include "csm/RamFileBodyIter.h"
#include "csm/ContainerBodyIter.h"
#include "csm/InjectIter.h"
#include "csm/GzipEncoder.h"
#include "csm/RangeBodyIter.h"
#include "csm/cdbEntries.h"
#include "csm/ContentDbase.h"
#include "csm/RamFiles.h"
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
	theRamFiles(0),
	theInjectionAlgorithm(ialgNone),
	theTdb(0), theInjGap(0), theInfProb(-1),
	theExtSel(0), thePfxSel(0),
	theId(anId),
	theEncodings(new int[codingEnd]),
	theMimeHeaders(0),
	theClientBehaviorSym(0),
	generateText(true) {
	theObjLifeCycle = new ObjLifeCycle;
}

ContentCfg::~ContentCfg() {
	delete theObjLifeCycle;
	delete[] theEncodings;
	delete theMimeHeaders;
	delete theRamFiles;
}

void ContentCfg::configure(const ContentSym &cfg) {
	theKind = cfg.kind();
	if (!theKind)
		theKind = DefaultContentCfgKind;

	ContType::Record(id(), kind());

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
		if (!theEmbedCont && theCdb->hasLinkOrPage())
			Comment << "error: content cfg `" << theKind << "': " <<
				"Content::may_contain is needed but missing" << endc << xexit;
	}

	if (const String &documentRoot = cfg.documentRoot()) {
		if (theCdb)
			Comment(0) << "error: Content document_root and content_db are mutually exclusive" << endc << xexit;

		theRamFiles = new RamFiles(documentRoot);
		theRamFiles->load();
		if (!theRamFiles->count()) {
			Comment(0) << "error: no valid files in Content " << theKind <<
				" document_root: " << documentRoot << endc << xexit;
		}
		Comment(1) << "fyi: loaded " << theRamFiles->count() << " files from " <<
			"Content " << theKind << " document_root " << documentRoot <<
			"; approximate RAM used: " << theRamFiles->ramSize() << endc;
	}

	//if (theEmbedCont && theCdb)
	//	Comment << "error: content cfg `" << theKind << "': "
	//		<< "cannot support containers together with content_db yet; "
	//		<< "do not use may_contain with content_db"
	//		<< endc << xexit;

	if (!theSize && !theCdb && !theRamFiles)
		Comment << "error: content cfg `" << theKind << "': "
			<< "has neither size distribution nor content_db; "
			<< "either one or both must be specified for "
			<< "Polygaph to know what object sizes this content type "
			<< "should generate"
			<< endc << xexit;

	configureInjections(cfg);

	configureEncodings(cfg);

	configureRndGeneration(cfg);

	theClientBehaviorSym = cfg.clientBehavior();
}

void ContentCfg::configureInjections(const ContentSym &cfg) {

	if (const String &objKind = cfg.injectObject()) {
		// convert external inject_object into internal InjectionAlgorithm
		static StringIdentifier knownKinds;
		if (!knownKinds.count()) {
			knownKinds.add("db_text", ialgTextBetweenMarkup);
			knownKinds.add("request_uri", ialgUriAtEnd);
		}

		const int id = knownKinds.lookup(objKind);
		if (id <= 0) {
			cerr << cfg.loc() << "unknown inject_object '" << objKind <<
				"'; known objects are:";
			for (StringIdentifier::Iter i = knownKinds.iterator(); i; ++i)
				cerr << ' ' << i.str();
			cerr << endl;
			exit(-2);
		}

		theInjectionAlgorithm = static_cast<InjectionAlgorithm>(id);
	}

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

		if (!theInjectionAlgorithm)
			theInjectionAlgorithm = ialgTextBetweenMarkup; // default for tdb
	}

	theInjGap = cfg.injectGap();
	
	const bool explicitProb = cfg.infectProb(theInfProb);
	configureMimeHeaders(cfg);

	// XXX: we should put all inject* fields into one PGL object
	// does it look like we need to inject?
	const char *need = 0;
	if (explicitProb)
		need = "infect_prob";
	if (theInjectionAlgorithm)
		need = "inject_object";
	if (theTdb)
		need = "inject_db";
	if (theInjGap)
		need = "inject_gap";
	if (!need)
		return;

	// do we have what it takes?
	const char *error = 0;
	if (!explicitProb)
		error = "lacks infect_prob";
	if (!theInjectionAlgorithm && !theTdb)
		error = "lacks either inject_object or inject_db";
	if (theTdb && theInjectionAlgorithm != ialgTextBetweenMarkup) {
		need = "inject_db";
		error = "uses a conflicting inject_object value";
	}
	if (theTdb && !theInjGap) {
		need = "inject_db";
		error = "lacks inject_gap";
	}
	if (theInjGap && !theTdb) {
		need = "inject_gap";
		error = "lacks inject_db";
	}
	if (!error)
		return;

	Comment << cfg.loc() << "error: content cfg '" << theKind << "' " <<
			"has " << need << " but " << error << endc << xexit;
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
				if (zlib::Supported) {
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

void ContentCfg::configureMimeHeaders(const ContentSym &cfg) {
	if (const ArraySym *const a = cfg.mimeHeaders())
		theMimeHeaders = new MimeHeadersCfg(*a);
}

void ContentCfg::configureRndGeneration(const ContentSym &cfg) {
	const String generator = cfg.generator();
	if (!generator.len() || generator == "random_text")
		generateText = true;
	else
	if (generator == "random_data")
		generateText = false;
	else {
		Comment << "error: unknown content generation method " << generator <<
			" in content cfg '" << theKind << "'; known methods are " <<
			"random_text and random_data" << endc << xexit;
	}
}

const String &ContentCfg::url_ext(int seed) const {
	return pickStr(theExtensions, theExtSel, seed);
}

const String &ContentCfg::url_pfx(int seed) const {
	return pickStr(thePrefixes, thePfxSel, seed);
}

double ContentCfg::repSizeMean() const {
	Assert(theSize || theCdb || theRamFiles);
	if (theSize)
		return RndDistrStat(theSize).mean();
	else
	if (theCdb)
		return theCdb->entrySizeMean();
	else
		return theRamFiles->fileSizeMean();
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

Size ContentCfg::calcRawRepSize(const ObjId &oid, Size *suffixSizePtr) const {
	Assert(theSize || theCdb || theRamFiles);
	// make sure both prefix and suffix fit
	const Size suffixSize = calcContentSuffixSize(oid);
	const Size extras = calcContentPrefixSize(oid) + suffixSize;
	if (suffixSizePtr)
		*suffixSizePtr = suffixSize;
	if (theSize) {
		const int seed = GlbPermut(contentHash(oid), rndRepSize);
		theSize->rndGen()->seed(seed);
		const double dh = theSize->trial();
		// prevent int overflows and leave room for headers
		Size sz = (int)MiniMax((double)extras, 
			ceil(dh), (double)INT_MAX - 100*1024);
		// paranoid sanity checks
		if (!Should(sz >= extras))
			sz = extras;
		if (!Should(sz >= 0))
			sz = 0;
		return sz;
	} else
	if (theCdb) {
		const int start = selectCdbStart(oid);
		CdbEntryPrnOpt opt;
		// assume that buf, injector, and rng are not needed
		opt.embed.model = theEmbedCont;
		opt.embed.container = oid;
		opt.sizeMax = Size(INT_MAX); // Size::Max();
		opt.entryOff = 0;
		return theCdb->entry(start)->size(opt) + extras;
	} else {
		const Size fileSize = ramFile(oid).body.len();
		return fileSize + extras;
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

bool ContentCfg::shouldInject(const ObjId &oid) const {
	if (theInfProb <= 0)
		return false;
	RndGen rng(GlbPermut(contentHash(oid), rndInjProb));
	return rng.event(theInfProb);
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

Size ContentCfg::calcContentSuffixSize(const ObjId &oid) const {
	if (theInjectionAlgorithm != ialgUriAtEnd)
		return 0; // no suffix configured at all

	if (!shouldInject(oid))
		return 0; // this object does not need a suffix

	WrBuf buf;
	return pourContentSuffix(oid, buf);
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

// A simpler implementation would overwrite the tail of the poured response,
// but that does not work for tiny responses, may malform some cdb entries,
// and may even overwrite already buffered headers.
Size ContentCfg::pourContentSuffix(const ObjId &oid, IOBuf &buf) const {
	if (theInjectionAlgorithm != ialgUriAtEnd)
		return 0; // OK, no suffix configured at all

	if (!shouldInject(oid))
		return 0; // OK, this object does not need a suffix

	// will need to be larger if we want to accomodate "long" foreign URIs
	char sfx[8*1024];
	ofixedstream os(sfx, sizeof(sfx));
	const Size size = pourUri(oid, os);
	if (size <= buf.spaceSize()) {
		buf.append(sfx, size);
		return size;
	}
	Should(size <= buf.capacity());
	return 0; // and wait for the buffer to drain
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
	const Size size = pourUri(oid, os);
	Should(size < buf.spaceSize()); // otherwise may be too big
	buf.appended(size);
	return size;
}

Size ContentCfg::pourUri(const ObjId &oid, ostream &os) const {
	// mimic Oid2Url() but do not use TheViservs and such, just indeces
	// if we need to allow truncated URIs, we can start with oid.hash()

	os.write(" u:", 3); // a prefix to ease debugging/tracing
	if (oid.foreignUrl()) {
		os << oid.foreignUrl();
	} else {
		os << hex << setfill('0');
		if (oid.secure())
			os.write("s/", 2);
		else
			os << oid.scheme() << '/';
		os << 'v' << setw(3) << oid.viserv() << '/' <<
			'w' << oid.world() << '/' <<
			't' << setw(2) << oid.type() << '/' <<
			'_' << setw(16) << oid.name();
	}
	os << ' ';
	os.flush();
	return static_cast<std::streamoff>(os.tellp()); // poured size
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

const RamFile &ContentCfg::ramFile(const ObjId &oid) const {
	Assert(theRamFiles && oid);
	const int index = (oid.name() - 1) % theRamFiles->count();
	return theRamFiles->fileAt(index);
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

const RndBuf &ContentCfg::rndBuf() const {
	return generateText ? RndText() : RndBinary();
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
			if (shouldInject(oid)) {
				InjectIter *inj = new InjectIter; // XXX: Farm these!
				inj->creator(this);
				inj->textDbase(theTdb);
				inj->gap(theInjGap);
				i->injector(inj);
			}
		}

		res = i;
	} else
	if (theRamFiles) {
		RamFileBodyIter *i = new RamFileBodyIter;
		i->file(ramFile(oid));
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
		Size suffixSize;
		const Size rawRepSize = calcRawRepSize(oid, &suffixSize);
		res->contentSize(rawRepSize, suffixSize);
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
