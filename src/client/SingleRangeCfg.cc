
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/SingleRangeCfg.h"
#include "pgl/SingleRangeSym.h"
#include "base/RangeGenStat.h"
#include "runtime/HttpPrinter.h"
#include "runtime/httpHdrs.h"
#include "runtime/httpText.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "csm/ContentCfg.h"

SingleRangeCfg::SingleRangeCfg():
	theFirstByteAbsolute(-1), theFirstByteRelative(-1),
	theLastByteAbsolute(-1), theLastByteRelative(-1),
	theSuffixLengthAbsolute(-1), theSuffixLengthRelative(-1) {
}

void SingleRangeCfg::configure(const SingleRangeSym &aSingleRange) {
	aSingleRange.firstByteAbsolute(theFirstByteAbsolute);
	aSingleRange.firstByteRelative(theFirstByteRelative);
	if (theFirstByteAbsolute >= 0 && theFirstByteRelative >= 0)
		cerr << "Both first_byte_pos_absolute and first_byte_pos_relative "
			<< "should not be specified for a SingleRange"
			<< endl << xexit;

	aSingleRange.lastByteAbsolute(theLastByteAbsolute);
	aSingleRange.lastByteRelative(theLastByteRelative);
	if (theLastByteAbsolute >= 0 && theLastByteRelative >= 0)
		cerr << "Both last_byte_pos_absolute and last_byte_pos_relative "
			<< "should not be specified for a SingleRange"
			<< endl << xexit;

	aSingleRange.suffixLengthAbsolute(theSuffixLengthAbsolute);
	aSingleRange.suffixLengthRelative(theSuffixLengthRelative);
	if (theSuffixLengthAbsolute >= 0 && theSuffixLengthRelative >= 0)
		cerr << "Both suffix_length_absolute and suffix_length_relative "
			<< "should not be specified for a SingleRange"
			<< endl << xexit;

	const bool first_byte_set = (theFirstByteAbsolute >= 0 || theFirstByteRelative >= 0);
	const bool last_byte_set = (theLastByteAbsolute >= 0 || theLastByteRelative >= 0);
	const bool suffix_length_set = (theSuffixLengthAbsolute >= 0 || theSuffixLengthRelative >= 0);

	if (!first_byte_set && !last_byte_set && !suffix_length_set)
		cerr << "No first byte, last byte or suffix length specified for a SingleRange"
			<< endl << xexit;

	if (last_byte_set && !first_byte_set)
		cerr << "First byte should be specified when last byte is set for a SingleRange"
			<< endl << xexit;

	if (first_byte_set && suffix_length_set)
		cerr << "Both suffix length and first byte should not be specified for a SingleRange"
			<< endl << xexit;
}

RangeCfg::RangesInfo SingleRangeCfg::makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const {
	RangeGenStat &rangeGenStat = TheStatPhaseMgr->rangeGenStat();
	Size sz;

	const bool putHeader = hp.putHeader(hfpRange);
        const int repSize = contentCfg.calcFullEntitySize(oid);
	if (theFirstByteAbsolute >= 0 || theFirstByteRelative >= 0) {
		const int firstByte = (theFirstByteAbsolute >= 0) ? theFirstByteAbsolute.byte() : (int)(theFirstByteRelative*repSize);
		if (putHeader)
			hp << firstByte << '-';

		int lastByte = -1;
		if (theLastByteAbsolute >= 0)
			lastByte = theLastByteAbsolute.byte();
		else
		if (theLastByteRelative >= 0)
			lastByte = (int)(theLastByteRelative*repSize);

		if (firstByte >= repSize)
			rangeGenStat.recordOneOffOver();
		if (firstByte > lastByte && lastByte >= 0) {
			rangeGenStat.recordAbsRelSwap();
			lastByte = firstByte;
		}

		if (lastByte >= 0) {
			if (putHeader)
				hp << lastByte;
			sz = lastByte - firstByte + 1;
		} else
			sz = repSize - firstByte;
	} else {
		int suffixLength;
		if (theSuffixLengthAbsolute >= 0)
			suffixLength = theSuffixLengthAbsolute.byte();
		else
			suffixLength = (int)(theSuffixLengthRelative*repSize);
		if (putHeader)
			hp << '-' << suffixLength;
		sz = suffixLength;
	}
	if (putHeader)
		hp << crlf;

	rangeGenStat.recordOneSize(sz);
	rangeGenStat.recordTotalSize(sz);

	RangesInfo res;
        res.theCount = 1;
        res.theTotalSize = sz;
	return res;
}
