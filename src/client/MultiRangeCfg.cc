
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/MultiRangeCfg.h"
#include "pgl/MultiRangeSym.h"
#include "xstd/rndDistrs.h"
#include "base/RndPermut.h"
#include "base/RangeGenStat.h"
#include "runtime/HttpPrinter.h"
#include "runtime/httpHdrs.h"
#include "runtime/httpText.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "csm/ContentCfg.h"

MultiRangeCfg::MultiRangeCfg():
	theGapLengthAbsolute(0), theGapLengthRelative(0),
	theFirstRangeStartAbsolute(0), theFirstRangeStartRelative(0),
	theRangeLengthAbsolute(0), theRangeLengthRelative(0),
	theRangeCount(0) {
}

void MultiRangeCfg::configure(const MultiRangeSym &aMultiRange) {
	theFirstRangeStartAbsolute = aMultiRange.first_range_start_absolute();
	theFirstRangeStartRelative = aMultiRange.first_range_start_relative();
	if (theFirstRangeStartAbsolute != 0 && theFirstRangeStartRelative != 0)
		cerr << "Both first_range_start_absolute and first_range_start_relative "
			<< "should not be specified for a MultiRange"
			<< endl << xexit;

	theRangeLengthAbsolute = aMultiRange.range_length_absolute();
	theRangeLengthRelative = aMultiRange.range_length_relative();
	if (theRangeLengthAbsolute != 0 && theRangeLengthRelative != 0)
		cerr << "Both range_length_absolute and range_length_relative "
			<< "should not be specified for a MultiRange"
			<< endl << xexit;
	if (theRangeLengthAbsolute == 0 && theRangeLengthRelative == 0)
		cerr << "range_length_absolute or range_length_relative "
			<< "should be specified for a MultiRange"
			<< endl << xexit;

	theRangeCount = aMultiRange.range_count();
	if (theRangeCount == 0)
		cerr << "range_count should be specified for a MultiRange"
			<< endl << xexit;

        RndGen *const rnd_gen = LclRndGen("multi_range_gap");
	if (theRangeLengthAbsolute != 0)
		theGapLengthAbsolute =
			new ExpDistr(rnd_gen, theRangeLengthAbsolute->mean());
	else
		theGapLengthRelative =
			new ExpDistr(rnd_gen, theRangeLengthRelative->mean());
}

RangeCfg::RangesInfo MultiRangeCfg::makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const {
	RangeGenStat &rangeGenStat = TheStatPhaseMgr->rangeGenStat();
	RangesInfo res;
	res.theCount = 0;
	res.theTotalSize = 0;

	const bool putHeader = hp.putHeader(hfpRange);
	const int repSize = contentCfg.calcFullEntitySize(oid);

	int first, last;

	if (theFirstRangeStartAbsolute != 0)
		first = theFirstRangeStartAbsolute->ltrial();
	else
	if (theFirstRangeStartRelative != 0)
		first = (int)(repSize * theFirstRangeStartRelative->trial());
	else
		first = calculateGap(repSize);

	const int count = theRangeCount->ltrial();
	for (int i = 0; i < count; ++i) {
		last = first;
		if (theRangeLengthAbsolute != 0)
			last += theRangeLengthAbsolute->ltrial();
		else
			last += (int)(repSize * theRangeLengthRelative->trial());
		if (first >= repSize) {
			rangeGenStat.recordTotalSizeOver();
			break;
		}
		if (putHeader) {
			hp << first << '-' << last;
			if (i < count - 1)
				hp << ',';
		}

		rangeGenStat.recordOneSize(last - first + 1);
		res.theTotalSize += last - first + 1;
		++res.theCount;

		first = last + calculateGap(repSize);
	}
	if (putHeader)
		hp << crlf;
	rangeGenStat.recordTotalSize(res.theTotalSize);

	return res;
}

int MultiRangeCfg::calculateGap(const int repSize) const {
	int gap;
	if (theGapLengthAbsolute != 0)
		gap = theGapLengthAbsolute->ltrial();
	else
		gap = (int)(repSize * theGapLengthRelative->trial());
	return gap;
}
