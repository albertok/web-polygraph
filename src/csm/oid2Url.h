
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_OID2URL_H
#define POLYGRAPH__CSM_OID2URL_H

#include "xstd/h/iosfwd.h"
#include "xstd/NetAddr.h"

class ObjId;
class ContentCfg;

extern ostream &Oid2Url(const ObjId &oid, ostream &os);
extern ostream &Oid2UrlHost(const ObjId &oid, const bool forcePortPrint, ostream &os);
extern ostream &Oid2UrlPath(const ObjId &oid, ostream &os);

// XXX: move to oidAlgos
extern NetAddr Oid2UrlHost(const ObjId &oid);
extern bool OidImpliesMarkup(const ObjId &oid, const ContentCfg *cfg = 0);
extern String Oid2AuthPath(const ObjId &oid);

#endif
