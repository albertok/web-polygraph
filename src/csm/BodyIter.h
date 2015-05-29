
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_BODYITER_H
#define POLYGRAPH__CSM_BODYITER_H

#include "xstd/Rnd.h"
#include "base/ObjId.h"

class WrBuf;
class ContentCfg;

class BodyIter {
	public:
		BodyIter();
		virtual ~BodyIter() {};
		virtual BodyIter *clone() const = 0;

		void contentCfg(const ContentCfg *cfg) { theContentCfg = cfg; }
		void oidCfg(const ObjId &anOid, int aHash) { theOid = anOid; theContentHash = aHash; }
		void contentSize(Size aContentSize);

		virtual void start(WrBuf *aBuf);
		virtual void stop() {}

		const ObjId &oid() const { return theOid; }
		const ContentCfg *contentCfg() const { return theContentCfg; }
		int contentHash() const { return theContentHash; }
		Size contentSize() const;
		virtual Size fullEntitySize() const;
		Size builtSize() const { return theBuiltSize; }
		Size offSeed() const { return theContentHash; }

		bool canPour() const; // can add to buffer and buffer has space
		bool pouredAll() const; // has someting to add to buffer
		virtual bool pour();  // fills the buffer; false if unrecoverable error
		virtual void putHeaders(ostream &os) const;

		operator void *() const { return pouredAll() ? 0 : (void*)-1; }

		void putBack();

	protected:
		void pourPrefix();
		virtual bool pourBody() = 0;
		virtual void calcContentSize() const;
		Size sizeLeft() const { return theContentSize.known() ?
			theContentSize - theBuiltSize : theContentSize; }

	protected:
		const ContentCfg *theContentCfg;
		ObjId theOid;  // XXX: replace with a pointer when ObjRec is removed
		WrBuf *theBuf;
		RndGen theRng;
		mutable Size theContentSize;
		int theContentHash;

		Size theBuiltSize;
};

#endif
