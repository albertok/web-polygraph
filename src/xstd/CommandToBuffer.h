
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_COMMANDTOBUFFER_H
#define POLYGRAPH__XSTD_COMMANDTOBUFFER_H

#include "xstd/h/iosfwd.h"

class String;

namespace xstd {

// runs system(cmd); returns command's stdout (or nil on errors)
extern stringstream *CommandToBuffer(const String &cmd);

}

#endif
