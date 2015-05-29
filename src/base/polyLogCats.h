
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_POLYLOGCATS_H
#define POLYGRAPH__BASE_POLYLOGCATS_H

// append-only table with well-known entry categories
typedef enum {
	lgcAll = 0, // applicable to all categories
	lgcCltSide,
	lgcSrvSide,

	lgcEnd
} LogEntryCat;


#endif
