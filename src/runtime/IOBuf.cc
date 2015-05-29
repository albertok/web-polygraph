
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "xstd/h/sstream.h"
#include "runtime/Farm.h"
#include "runtime/LogComment.h"
#include "runtime/IOBuf.h"

class BufPool: public Farm<char> {
	public:
		BufPool(Size aBufCap = -1);
		virtual ~BufPool() { while (!empty()) destroy(get()); } // g++ bug: {}

		void bufCap(Size aBufCap);
		Size bufCap() const { return theBufCap; }

	protected:
		virtual char *gen();
		virtual void destroy(char *b) { delete[] b; }
		virtual void overflow(char *b) { push(b); }

	protected:
		Size theBufCap;
		Size theRepLvl; // when to report growth
};

class RndBuf {
	public:
		RndBuf(Size aCapacity);
		~RndBuf();

		Size capacity() const { return theCapacity; }

		const char *bufAt(Size off) const;

	protected:
		char *theBuf;
		Size theCapacity;
};

static BufPool *TheBufPool = 0;
static RndBuf *TheRndBuf = 0;     // pre-filled to rnd-ize other buffers


/* IOBuf */

IOBuf::IOBuf(): theBuf(0), theCapacity(0), 
	theOutOff(0), theInOff(0), isZippable(true) {
	Assert(TheBufPool);
	theCapacity = TheBufPool->bufCap();
	Assert(theCapacity > 0);
	// note: the buffer is created in "zipped" state
}

IOBuf::IOBuf(Size aCapacity): theBuf(0), theCapacity(aCapacity),
	theOutOff(0), theInOff(0), isZippable(false) {
	Assert(theCapacity > 0);
	theBuf = new char[theCapacity];
}

IOBuf::~IOBuf() {
	if (isZippable) {
		zip();
	} else {
		delete[] theBuf;
		theBuf = 0;
	}
}

void IOBuf::append(const char *buf, Size sz) {
	Assert(spaceSize() >= sz);
	if (sz > 0) {
		memcpy(space(), buf, sz);
		appended(sz);
	}
}

void IOBuf::copyContent(IOBuf &buf, Size maxSize) const {
	buf.append(content(), Min(contSize(), maxSize));
}

Size IOBuf::RandomOffset(Size seed, Size off) {
	if (!Should(seed >= 0))
		seed = 0;
	const Size cap = TheRndBuf->capacity();
	// prevent int overflow; seed and off could be large
	return ((seed % cap) + (off % cap)) % cap;
}

void IOBuf::RandomFill(ostream &os, Size rndOffset, Size sz) {
	if (!Should(rndOffset >= 0))
		rndOffset = 0;
	os.write(TheRndBuf->bufAt(rndOffset % TheRndBuf->capacity()), sz);
}

void IOBuf::appendRnd(Size rndOffset, Size sz) {
	Assert(spaceSize() >= sz);
	ofixedstream os(space(), spaceSize());
	RandomFill(os, rndOffset, sz);
	appended((std::streamoff)os.tellp());
}

Size IOBuf::appendRndUpTo(Size rndOff, Size size) {
	Assert(size >= 0);
	const Size sz = Min(spaceSize(), size);
	appendRnd(rndOff, sz); // stuff with random data
	return sz;
}

void IOBuf::saveState(IOBufState &state) const {
	state.theBuf = theBuf;
	state.theCapacity = theCapacity;
	state.theOutOff = theOutOff;
	state.theInOff = theInOff;
}

void IOBuf::restoreState(const IOBufState &state) {
	// some things should not change because we cannot restore them
	Assert(theBuf == state.theBuf);
	Assert(theCapacity == state.theCapacity);
	theOutOff = state.theOutOff;
	theInOff = state.theInOff;
}

void IOBuf::pack() {
	if (empty()) {
		reset();
	} else
	if (theOutOff) {
		memcpy(theBuf, content(), contSize());
		theInOff -= theOutOff;
		theOutOff = 0;
	}
}

// return [empty] buffer to shared area
void IOBuf::zip() {
	if (!isZippable)
		return;

	if (theBuf) { // could be zipped already
		TheBufPool->put(theBuf);
		theBuf = 0;
	}
}

void IOBuf::unzip() {
	if (!theBuf) {
		Assert(isZippable);
		theBuf = TheBufPool->getDirty();
		Assert(theBuf);
	}
}


/* WrBuf */

void WrBuf::overwrite(int offset, const char *buf, Size sz) {
	Assert(0 <= offset && offset <= contSize());
	if (sz > 0)
		memcpy(theBuf+offset, buf, sz);
}


/* BufPool */

BufPool::BufPool(Size aBufCap):
	theBufCap(aBufCap), theRepLvl(Size::MB(20)) {
	Assert(TheRndBuf);
}

void BufPool::bufCap(Size aBufCap) {
	Assert(empty() && !outLevel());
	theBufCap = aBufCap;
}

char *BufPool::gen() {
	Assert(theBufCap > 0);
	char *b = new char[theBufCap];

	const Size poolSize = outLevel()*theBufCap;
	if (poolSize >= theRepLvl) {
		Comment(1) << "buffer pool grew to " 
			<< outLevel() << " x " << theBufCap << " = " << poolSize << endc;
		theRepLvl += Min(theRepLvl, Size::MB(10));
	}

	return b;
}


/* RndBuf */

RndBuf::RndBuf(Size aCapacity): theBuf(0), theCapacity(aCapacity) {
	const Size cap = theCapacity * 2; // more to allow random offsets
	theBuf = new char[cap];

	RndGen rng;
	for (int i = 0; i < theCapacity; ++i) {
		const char c = (char)rng(33, 127);
		// to assist with HTML parsing and HTTP header generatio,
		// substitute '<' and '"' with a space
		theBuf[i] = (c == '<' || c == '"') ? ' ' : c; 
	}
	memcpy(theBuf+theCapacity, theBuf, theCapacity);
}

RndBuf::~RndBuf() {
	delete [] theBuf;
	theBuf = 0;
}

const char *RndBuf::bufAt(Size off) const {
	Assert(off < theCapacity);
	return theBuf + off;
}

/* initialization */

int IOBufInit::TheUseCount = 0;

void IOBufInit::init() {
	const Size bufCap = 16*1024;  // XXX: hardcoded IO buf capacity

	TheRndBuf = new RndBuf(bufCap);
	TheBufPool = new BufPool(bufCap);
}

void IOBufInit::clean() {
	delete TheBufPool; TheBufPool = 0;
	delete TheRndBuf; TheRndBuf = 0;
}

