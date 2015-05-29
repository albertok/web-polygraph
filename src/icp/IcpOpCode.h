
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__ICP_ICPOPCODE_H
#define POLYGRAPH__ICP_ICPOPCODE_H

enum IcpOpCode {
	icpInvalid     =   0, 
	icpQuery       =   1,
	icpHit         =   2, 
	icpMiss        =   3,
	icpError       =   4,
	icpMissNoFetch =  21,
	icpDenied      =  22,

	icpTimeout     = 127 // not a real code, but convenient
};

/*
   http://www.squid-cache.org/Doc/rfc2186.txt

       0    ICP_OP_INVALID
       1    ICP_OP_QUERY
       2    ICP_OP_HIT
       3    ICP_OP_MISS
       4    ICP_OP_ERR
     5-9    UNUSED
      10    ICP_OP_SECHO
      11    ICP_OP_DECHO
   12-20    UNUSED
      21    ICP_OP_MISS_NOFETCH
      22    ICP_OP_DENIED
      23    ICP_OP_HIT_OBJ
*/

#endif
