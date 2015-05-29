
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iostream.h"
#include <limits.h>
#include "xstd/h/math.h"

#include "xstd/Assert.h"
#include "xstd/BigSize.h"


/* static creators */

BigSize BigSize::MB(int n) {
	const int factor = INT_MAX/(1024*1024); // MBs in a chunk
	Assert(factor > 0);
	BigSize sz;
	sz.theCnt = n / factor; // chunks in n
	sz.theAcc = (n % factor) * (1024*1024); // leftovers
	return sz;
}

BigSize BigSize::MBd(double mb) {
	const int factor = INT_MAX/(1024*1024); // MBs in a chunk
	const double n = mb / factor; // chunks in mb;
	double fullCh = 0;
	const double partialCh = modf(n, &fullCh);
	Assert(INT_MIN <= fullCh && fullCh < INT_MAX);
	BigSize sz;
	sz.theCnt = (int)fullCh;
	sz.theAcc = (int)(INT_MAX*partialCh);
	return sz;
}

BigSize BigSize::Byted(double b) {
	return MBd(b/(1024*1024));
}

/* dynamic methods */

BigSize::BigSize(int bytes): theCnt(0), theAcc(bytes) {
}

BigSize &BigSize::operator +=(const BigSize &bsz) {
	theCnt += bsz.theCnt;
	addBytes(bsz.theAcc);
	return *this;
}

void BigSize::addBytes(int sz) {
	if (sz < 0) {
		subBytes(-sz);
		return;
	}

	const int safe = INT_MAX - sz;
	if (safe < theAcc) { // overflow
		theAcc -= safe;
		theCnt++;
	} else {
		theAcc += sz;
	}
}

BigSize &BigSize::operator -=(const BigSize &bsz) {
	theCnt -= bsz.theCnt;
	subBytes(bsz.theAcc);
	return *this;
}

void BigSize::subBytes(int sz) {
	if (sz <= 0) {
		addBytes(-sz);
		return;
	}

	const int safe = sz + INT_MIN;

	if (theAcc < safe) { // underflow
		theAcc += INT_MAX - sz;
		theCnt--;
	} else {
		theAcc -= sz;
	}
}

BigSize &BigSize::operator *=(double f) {
	const double d = byted() * f;
	theCnt = (int)floor(d / INT_MAX);
	theAcc = (int)floor(d - theCnt*(double)INT_MAX);
	return *this;
}

double BigSize::operator /=(const BigSize &sz) const {
	return byted()/sz.byted();
}

int BigSize::byte() const {
	Assert(theCnt == 0); // otherwise may overflow
	return theAcc;
}

double BigSize::byted() const {
	return theCnt*(double)INT_MAX + theAcc;
}

ostream &BigSize::print(ostream &os) const {

	if (theCnt == 0) {
		if (theAcc == 0)
			os << theAcc;
		else
		if (theAcc < 100*1024)
			os << theAcc << "Byte";
		else
		if (theAcc < 1024*1024)
			os << theAcc/1024. << "KB";
		else
		if (theAcc < 1024*1024*1024)
			os << theAcc/(1024*1024.) << "MB";
		else
			os << theAcc/(1024*1024*1024.) << "GB";
	} else {
		os << byted()/(1024*1024*1024.) << "GB";
	}

	return os;
}
