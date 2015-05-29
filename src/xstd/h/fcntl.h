
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_FCNTL_H
#define POLYGRAPH__XSTD_H_FCNTL_H

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

// make 'rw' permissions flag to use in open(2)
#ifndef S_IRWMOST
#	if defined(S_IRUSR) && defined(S_IRGRP) & defined(S_IROTH)
#		define S_IRWMOST (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)
#   else
#		define S_IRWMOST 0000666
#	endif
#endif


#endif
