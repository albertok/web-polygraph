
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_POLYLOGTAGS_H
#define POLYGRAPH__BASE_POLYLOGTAGS_H

// append-only table with well-known binary log tags
typedef enum { 
	lgNone = 0,

	// special "system" tags
	lgProgress,
	lgComment,
	lgGroupId,
	lgEndOfSysTags = 255,

	// user tags
	lgStatCycleRec,
	lgStatPhaseBeg,
	lgStatPhaseEnd,
	lgStatPhaseRec,
	lgErrorStrs,
	lgErrorStat,

	lgAppCfg,
	lgCltCfg,
	lgSrvCfg,
	lgSrvRepCfg,

	lgAppState,
	lgCltState,
	lgSrvState,
	lgSrvRepState,

	lgStatsSmplBeg,
	lgStatsSmplEnd, // not used

	lgContTypeKinds,

	lgXactStats,
	lgUnused_former_PglCfg,
	lgPglCfg,

	// end marker
	lgEnd
} LogEntryTag;

// magic labels for binary logs (magic labels are not magic numbers!)
typedef enum {
	lgMagic1 = 0x506f6c79,
	lgMagic2 = 0x4c6f672e
} LogMagicLabel;


#endif
